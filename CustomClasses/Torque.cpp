#include "Torque.h"
#include "structs.h"
#include <stdio.h>
#include <Psapi.h>

//Con::printf
PrintfFn Printf;

DWORD ImageBase = NULL;
DWORD ImageSize = NULL;

//StringTable::insert
StringTableInsertFn StringTableInsert;
//Namespace::find
NamespaceFindFn NamespaceFind;
//Namespace::createLocalEntry
NamespaceCreateLocalEntryFn NamespaceCreateLocalEntry;
//Namespace::trashCache
NamespaceTrashCacheFn NamespaceTrashCache;

//Dictionary::add
DictionaryAddFn DictionaryAdd;
DictionaryAddVariableFn DictionaryAddVariable;
DictionaryGetVariableFn DictionaryGetVariable;
DictionaryEntrySetStringValueFn DictionaryEntrySetStringValue;
prependDollarFn prependDollar;

EvaluateFn Evaluate;
CodeBlockConstructorFn CodeBlockConstructor;
CodeBlockCompileExecFn CodeBlockCompileExec;
dAllocFn dAlloc;
dFreeFn dFree;

// TODO: use tork allocators
void* operator new (size_t size) { return dAlloc(size); }
void* operator new[] (size_t size) { return dAlloc(size); }

void operator delete (void* buf) { dFree(buf); }
void operator delete[](void* buf) { dFree(buf); }

//Set the module start and length
void InitScanner(const char* moduleName)
{
	//Retrieve information about the module
	HMODULE module = GetModuleHandleA(moduleName);
	if (!module) return;

	MODULEINFO info;
	GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(MODULEINFO));

	//Store relevant information
	ImageBase = (DWORD)info.lpBaseOfDll;
	ImageSize = info.SizeOfImage;
}

//Compare data at two locations for equality
bool CompareData(PBYTE data, PBYTE pattern, char* mask)
{
	//Iterate over the data, pattern and mask in parallel
	for (; *mask; ++data, ++pattern, ++mask)
	{
		//And check for equality at each unmasked byte
		if (*mask == 'x' && *data != *pattern)
			return false;
	}

	return (*mask) == NULL;
}

//Find a pattern in memory
DWORD FindPattern(DWORD imageBase, DWORD imageSize, PBYTE pattern, char* mask)
{
	//Iterate over the image
	for (DWORD i = imageBase; i < imageBase + imageSize; i++)
	{
		//And check for matching pattern at every byte
		if (CompareData((PBYTE)i, pattern, mask))
			return i;
	}

	return 0;
}

//Scan the module for a pattern
DWORD ScanFunc(char* pattern, char* mask)
{
	//Just search for the pattern in the module
	return FindPattern(ImageBase, ImageSize - strlen(mask), (PBYTE)pattern, mask);
}

//Change a byte at a specific location in memory
void PatchByte(BYTE* location, BYTE value)
{
	//Remove protection
	DWORD oldProtection;
	VirtualProtect(location, 1, PAGE_EXECUTE_READWRITE, &oldProtection);

	//Change value
	*location = value;

	//Restore protection
	VirtualProtect(location, 1, oldProtection, &oldProtection);
}

Namespace::Entry* InsertFunction(const char* nameSpace, const char* name, bool caseSens) {
	Namespace* ns = NULL;
	if (nameSpace) {
		ns = NamespaceFind(StringTableInsert(nameSpace, caseSens), 0);
	}
	else {
		ns = mGlobalNamespace;
	}

	Namespace::Entry* entry = NamespaceCreateLocalEntry(ns, StringTableInsert(name, caseSens));
	NamespaceTrashCache();
	return entry;
}

//Register a torquescript function that returns a string. The function must look like this:
//const char* func(DWORD* obj, int argc, const char* argv[])
void AddFunction(const char* ns, const char* name, Namespace::StringCallback cb, const char* usage, int minArgs, int maxArgs, bool caseSens)
{
	Namespace::Entry* func = InsertFunction(ns, name, caseSens);
	func->mUsage = usage;
	func->mMaxArgs = maxArgs;
	func->mMinArgs = minArgs;
	func->mType = Namespace::Entry::StringCallbackType;
	func->cb.mStringCallbackFunc = cb;
}

//Register a torquescript function that returns an int. The function must look like this:
//int func(DWORD* obj, int argc, const char* argv[])
void AddFunction(const char* ns, const char* name, Namespace::IntCallback cb, const char* usage, int minArgs, int maxArgs, bool caseSens)
{	
	Namespace::Entry* func = InsertFunction(ns, name, caseSens);
	func->mUsage = usage;
	func->mMaxArgs = maxArgs;
	func->mMinArgs = minArgs;
	func->mType = Namespace::Entry::IntCallbackType;
	func->cb.mIntCallbackFunc = cb;
}

//Register a torquescript function that returns a float. The function must look like this:
//float func(DWORD* obj, int argc, const char* argv[])
void AddFunction(const char* ns, const char* name, Namespace::FloatCallback cb, const char* usage, int minArgs, int maxArgs, bool caseSens)
{
	Namespace::Entry* func = InsertFunction(ns, name, caseSens);
	func->mUsage = usage;
	func->mMaxArgs = maxArgs;
	func->mMinArgs = minArgs;
	func->mType = Namespace::Entry::FloatCallbackType;
	func->cb.mFloatCallbackFunc = cb;
}

//Register a torquescript function that returns nothing. The function must look like this:
//void func(DWORD* obj, int argc, const char* argv[])
void AddFunction(const char* ns, const char* name, Namespace::VoidCallback cb, const char* usage, int minArgs, int maxArgs, bool caseSens)
{
	Namespace::Entry* func = InsertFunction(ns, name, caseSens);
	func->mUsage = usage;
	func->mMaxArgs = maxArgs;
	func->mMinArgs = minArgs;
	func->mType = Namespace::Entry::VoidCallbackType;
	func->cb.mVoidCallbackFunc = cb;
}

//Register a torquescript function that returns a bool. The function must look like this:
//bool func(DWORD* obj, int argc, const char* argv[])
void AddFunction(const char* ns, const char* name, Namespace::BoolCallback cb, const char* usage, int minArgs, int maxArgs, bool caseSens)
{
	Namespace::Entry* func = InsertFunction(ns, name, caseSens);
	func->mUsage = usage;
	func->mMaxArgs = maxArgs;
	func->mMinArgs = minArgs;
	func->mType = Namespace::Entry::BoolCallbackType;
	func->cb.mBoolCallbackFunc = cb;
}

//Expose an integer variable to torquescript
void AddVariable(const char* name, int* data)
{
	DictionaryAddVariable(&gEvalState->globalVars, StringTableInsert(name, 0), 4, data);
}

//Expose a boolean variable to torquescript
void AddVariable(const char* name, bool* data)
{
	DictionaryAddVariable(&gEvalState->globalVars, StringTableInsert(name, 0), 6, data);
}

//Expose a float variable to torquescript
void AddVariable(const char* name, float* data)
{
	DictionaryAddVariable(&gEvalState->globalVars, StringTableInsert(name, 0), 8, data);
}

//Expose a string variable to torquescript
void AddVariable(const char* name, char* data)
{
	DictionaryAddVariable(&gEvalState->globalVars, StringTableInsert(name, 0), 10, data);
}

//Get a global variable
const char* GetGlobalVariable(const char* name) {
	return DictionaryGetVariable(&gEvalState->globalVars, StringTableInsert(name, 0));
}

//Set a global variable
void SetGlobalVariable(const char* name, const char* val) {
	Dictionary::Entry* entry = DictionaryAdd(&gEvalState->globalVars, StringTableInsert(prependDollar(name), 0));
	DictionaryEntrySetStringValue(entry, val);
}

//Evaluate a torquescript string in global scope
const char* Eval(const char* str)
{
	CodeBlock* block = (CodeBlock*)dAlloc(0x40);
	block = CodeBlockConstructor(block);
	return CodeBlockCompileExec(block, NULL, str, false);
}

//Initialize the Torque Interface
bool InitTorqueStuff()
{
	//Init the scanner

	Printf = (PrintfFn)(ImageBase + 0x37BC0);
	if (!Printf)
		return false;
	StringTableInsert = (StringTableInsertFn)(ImageBase + 0x053910);
	NamespaceFind = (NamespaceFindFn)(ImageBase + 0x41060);
	mGlobalNamespace = NamespaceFind(NULL, NULL);
	gEvalState = (ExprEvalState*)(ImageBase + 0x384800);
	NamespaceCreateLocalEntry = (NamespaceCreateLocalEntryFn)(ImageBase + 0x0415A0);
	NamespaceTrashCache = (NamespaceTrashCacheFn)(ImageBase + 0x3D7C0);
	DictionaryAdd = (DictionaryAddFn)(ImageBase + 0x40720);
	DictionaryAddVariable = (DictionaryAddVariableFn)(ImageBase + 0x40D20);
	DictionaryGetVariable = (DictionaryGetVariableFn)(ImageBase + 0x40BC0);
	DictionaryEntrySetStringValue = (DictionaryEntrySetStringValueFn)(ImageBase + 0x40C10);
	prependDollar = (prependDollarFn)(ImageBase + 0x36ED0);
	dFree = (dFreeFn)(ImageBase + 0x176E40);
	dAlloc = (dAllocFn)(ImageBase + 0x178160);
	CodeBlockConstructor = (CodeBlockConstructorFn)(ImageBase + 0x335B0);
	CodeBlockCompileExec = (CodeBlockCompileExecFn)(ImageBase + 0x340A0);
	return true;
}