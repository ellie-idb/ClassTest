#include "TestClass.h"

ConsoleMethod(void, testMethod)
{
	TestClass* o = (TestClass*)obj;
	Printf("Member method called");
	Printf("%d", o->testInt);
	/* Walk the class list and print it out as a proof of concept */
	for (AbstractClassRep* walk = AbstractClassRep::getClassList(); walk; walk = walk->nextClass)
	{
		Printf("%s", walk->mNamespace->mName);
		Printf("%s", walk->mClassName);
	}
}

/*
 * Since we can't inherit from SimObject directly, we need to patch our vTable to have the pointers
 * to what is actually in the SimObject vtable.
 * This can be fixed later
 */
TestClass::TestClass()
{
	Printf("Fixing vTable");
	testInt = 0;
	fixVTable();
}

TestClass::~TestClass()
{
	Printf("Goodbye");
}

void TestClass::initPersistFields()
{
	addGroup("Test");
	addField("test", TypeInt, Offset(testInt, TestClass), 1, 0, "Test");
	endGroup("Test");
}

void TestClass::consoleInit()
{
	Printf("TestClass::consoleInit");
	Namespace* ns = TestClass::getStaticClassRep()->mNamespace;
	RegisterInternalMethod(ns, testMethod, "() - Test method", 2, 2);
}

static int simObjectVFTSize = (17 * sizeof(void*));
static bool vftFixed = false;
void TestClass::fixVTable()
{
	if (vftFixed) return;

	// vtables are always located in .RDATA, so we need to patch them to enable R/W
	void* ourVtable = *(void**)this;
	DWORD oldProtection;
	VirtualProtect(ourVtable, simObjectVFTSize, PAGE_READWRITE, &oldProtection);
	// Ignore getClassRep() and our dtor, only copy the rest
	// If you want to override any other functions, you *need* to do it here and restore them
	memcpy((void*)((DWORD)ourVtable + 0x8), (void*)((DWORD)pSimObjectVTable + 0x8), 15 * sizeof(void*));
	VirtualProtect(ourVtable, simObjectVFTSize, oldProtection, &oldProtection);
	vftFixed = true;
}


IMPLEMENT_CONOBJECT(TestClass);