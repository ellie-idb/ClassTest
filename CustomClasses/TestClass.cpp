#include "TestClass.h"

ConsoleMethod(void, testMethod)
{
	Printf("Member method called");
	AbstractClassRep* a = *(AbstractClassRep**)(ImageBase + 0x3C5D18);
	for (AbstractClassRep* walk = a; walk; walk = walk->nextClass)
	{
		Printf("%s", walk->mNamespace->mName);
		Printf("%s", walk->mClassName);
	}
}

TestClass::TestClass()
{
	Printf("Fixing vTable");
	fixVTable();
}

TestClass::~TestClass()
{
	Printf("Goodbye");
}

void TestClass::initPersistFields() {}

void TestClass::consoleInit()
{
	Printf("TestClass::consoleInit");
	const char* func = StringTableInsert("testMethod", true);
	const char* name = StringTableInsert("TestClass", true);

	Namespace::Entry* entry = InsertFunction(name, func, true);
	entry->mUsage = "() - Test";
	entry->mMaxArgs = 2;
	entry->mMinArgs = 2;
	entry->mType = Namespace::Entry::VoidCallbackType;
	entry->cb.mVoidCallbackFunc = ts_testMethod;
}

static bool vftFixed = false;
void TestClass::fixVTable()
{
	if (vftFixed) return;
	
	void* ourVtable = *(void**)this;
	DWORD oldProtection;
	VirtualProtect(ourVtable, 17 * sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtection);
	memcpy((void*)((DWORD)ourVtable + 0x8), (void*)((DWORD)pSimObjectVTable + 0x8), 15 * sizeof(void*));
	VirtualProtect(ourVtable, 17 * sizeof(void*), oldProtection, &oldProtection);
	vftFixed = true;
}


IMPLEMENT_CONOBJECT(TestClass);