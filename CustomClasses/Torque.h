#ifndef _TORQUE_H_
#define _TORQUE_H_

#include <Windows.h>
#include "structs.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Macros

//Typedef an engine function to use it later
#define BLFUNC(returnType, convention, name, ...)         \
	typedef returnType (convention*name##Fn)(__VA_ARGS__); \
	static name##Fn name;

//Typedef an exported engine function to use it later
#define BLFUNC_EXTERN(returnType, convention, name, ...)  \
	typedef returnType (convention*name##Fn)(__VA_ARGS__); \
	extern name##Fn name;

//Search for an engine function in blockland
#define BLSCAN(target, pattern, mask)            \
	target = (target##Fn)ScanFunc(pattern, mask); \
	if(target == NULL)                             \
		Printf("BaseBlocklandDLL | Cannot find function "#target"!");

#define ConsoleMethod(type, function) type ts_##function(SimObject* obj, int argc, const char* argv[])
#define RegisterMethod(name, description, argmin, argmax) AddFunction(NULL, #name, ts_##name, description, argmin, argmax);
#define RegisterMethodNS(namespace, name, desc, argmin, argmax) AddFunction(#namespace, #name, ts_##name, desc, argmin, argmax);
#define RegisterInternalMethod(namespace, name, desc, argmin, argmax) AddInternalFunction(namespace, #name, ts_##name, desc, argmin, argmax, true);

//Start of the Blockland.exe module in memory
extern DWORD ImageBase;
extern DWORD ImageSize;

//StringTable pointer
// static DWORD StringTable;

void InitScanner(const char* moduleName);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Engine function declarations

//Con::printf
BLFUNC_EXTERN(void, , Printf, const char* format, ...);

BLFUNC_EXTERN(int, , RTDynamicCast, DWORD, DWORD, DWORD, DWORD, DWORD);

BLFUNC_EXTERN(void, , ACR_Initialize);

BLFUNC_EXTERN(const char*, __stdcall, StringTableInsert, const char* val, const bool caseSensitive);
//Namespace::find
BLFUNC_EXTERN(Namespace*, __fastcall, NamespaceFind, const char* name, const char* package);
//Namespace::createLocalEntry
BLFUNC_EXTERN(Namespace::Entry*, __thiscall, NamespaceCreateLocalEntry, Namespace * this_, const char* name);
//Namespace::trashCache
BLFUNC_EXTERN(void, , NamespaceTrashCache);

//Dictionary::add
BLFUNC_EXTERN(Dictionary::Entry*, __thiscall, DictionaryAdd, Dictionary * this_, const char* name);
BLFUNC_EXTERN(void, __thiscall, DictionaryAddVariable, Dictionary * this_, const char* name, int type, void* ptr);
BLFUNC_EXTERN(const char*, __thiscall, DictionaryGetVariable, Dictionary * this_, const char* name);
BLFUNC_EXTERN(void, __thiscall, DictionaryEntrySetStringValue, Dictionary::Entry * this_, const char* val);
BLFUNC_EXTERN(const char*, , prependDollar, const char* var);

//Executing code and calling torquescript functions
BLFUNC_EXTERN(const char*, , Evaluate, const char* string, bool echo, const char* fileName);
BLFUNC_EXTERN(CodeBlock*, __thiscall, CodeBlockConstructor, CodeBlock * this_);
BLFUNC_EXTERN(const char*, __thiscall, CodeBlockCompileExec, CodeBlock * this_, const char* fileName, const char* script, bool noCalls);
BLFUNC_EXTERN(void*, __fastcall, dAlloc, size_t size);
BLFUNC_EXTERN(void, __fastcall, dFree, void* buf);

BLFUNC_EXTERN(void, __fastcall, ConsoleObjectAddGroup, const char* groupName, const char* groupDocs);
BLFUNC_EXTERN(void, __fastcall, ConsoleObjectEndGroup, const char* groupName);
BLFUNC_EXTERN(void,  __fastcall, ConsoleObjectAddField, const char* fieldName, U32 fieldType, size_t fieldOffset, U32 elementCount, EnumTable* table, const char* fieldDocs);
BLFUNC_EXTERN(void, __fastcall, ConsoleObjectAddFieldV, const char* pFieldName, U32 fieldType, size_t fieldOffset, TypeValidator * validator);
BLFUNC_EXTERN(void, __fastcall, ConsoleObjectAddDeprecatedField, const char* fieldName);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Public functions

Namespace::Entry* InsertFunction(const char* nameSpace, const char* name, bool caseSens = false);
//Scan the module for a pattern
DWORD ScanFunc(char* pattern, char* mask);

//Change a byte at a specific location in memory
void PatchByte(BYTE* location, BYTE value);


//Register a torquescript function that returns a string. The function must look like this:
//const char* func(DWORD* obj, int argc, const char* argv[])
void AddInternalFunction(Namespace* nameSpace, const char* name, Namespace::StringCallback callBack, const char* usage, int minArgs, int maxArgs, bool caseSens = false);

//Register a torquescript function that returns an int. The function must look like this:
//int func(DWORD* obj, int argc, const char* argv[])
void AddInternalFunction(Namespace* nameSpace, const char* name, Namespace::IntCallback callBack, const char* usage, int minArgs, int maxArgs, bool caseSens = false);

//Register a torquescript function that returns a float. The function must look like this:
//float func(DWORD* obj, int argc, const char* argv[])
void AddInternalFunction(Namespace* nameSpace, const char* name, Namespace::FloatCallback callBack, const char* usage, int minArgs, int maxArgs, bool caseSens = false);

//Register a torquescript function that returns nothing. The function must look like this:
//void func(DWORD* obj, int argc, const char* argv[])
void AddInternalFunction(Namespace* nameSpace, const char* name, Namespace::VoidCallback callBack, const char* usage, int minArgs, int maxArgs, bool caseSens = false);

//Register a torquescript function that returns a bool. The function must look like this:
//bool func(DWORD* obj, int argc, const char* argv[])
void AddInternalFunction(Namespace* nameSpace, const char* name, Namespace::BoolCallback callBack, const char* usage, int minArgs, int maxArgs, bool caseSens = false);

//Register a torquescript function that returns a string. The function must look like this:
//const char* func(DWORD* obj, int argc, const char* argv[])
void AddFunction(const char* nameSpace, const char* name, Namespace::StringCallback callBack, const char* usage, int minArgs, int maxArgs, bool caseSens = false);

//Register a torquescript function that returns an int. The function must look like this:
//int func(DWORD* obj, int argc, const char* argv[])
void AddFunction(const char* nameSpace, const char* name, Namespace::IntCallback callBack, const char* usage, int minArgs, int maxArgs, bool caseSens = false);

//Register a torquescript function that returns a float. The function must look like this:
//float func(DWORD* obj, int argc, const char* argv[])
void AddFunction(const char* nameSpace, const char* name, Namespace::FloatCallback callBack, const char* usage, int minArgs, int maxArgs, bool caseSens = false);

//Register a torquescript function that returns nothing. The function must look like this:
//void func(DWORD* obj, int argc, const char* argv[])
void AddFunction(const char* nameSpace, const char* name, Namespace::VoidCallback callBack, const char* usage, int minArgs, int maxArgs, bool caseSens = false);

//Register a torquescript function that returns a bool. The function must look like this:
//bool func(DWORD* obj, int argc, const char* argv[])
void AddFunction(const char* nameSpace, const char* name, Namespace::BoolCallback callBack, const char* usage, int minArgs, int maxArgs, bool caseSens = false);

//Expose an integer variable to torquescript
void AddVariable(const char* name, int* data);

//Expose a boolean variable to torquescript
void AddVariable(const char* name, bool* data);

//Expose a float variable to torquescript
void AddVariable(const char* name, float* data);

//Expose a string variable to torquescript
void AddVariable(const char* name, char* data);

//Get a global variable
const char* GetGlobalVariable(const char* name);
//Set a global variable
void SetGlobalVariable(const char* name, const char* val);

//Evaluate a torquescript string in global scope
const char* Eval(const char* str);

//Initialize the Torque Interface
bool InitTorqueStuff();

#endif