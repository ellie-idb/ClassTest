#include <polyhook2/Detour/x86Detour.hpp>
#include <polyhook2/CapstoneDisassembler.hpp>
#include "Torque.h"
#include "TestClass.h"

#pragma comment(lib, "PolyHook_2.lib")
#pragma comment(lib, "capstone_dll.lib")

PLH::x86Detour* RTDynamicDetour = NULL;
uint64_t RTDynamicTramp = NULL;
RTDynamicCastFn RTDynamicCast = NULL;

PLH::x86Detour* ACR_InitializeDetour = NULL;
uint64_t ACR_InitializeTramp = NULL;
ACR_InitializeFn ACR_Initialize = NULL;

void* pSimObjectVTable;

void h_ACRInit()
{
	AbstractClassRep::registerClassRep(TestClass::getStaticClassRep());
	Printf("initializing ACR now");
	PLH::FnCast(ACR_InitializeTramp, ACR_Initialize)();
}

int h_RTDynamicCast(DWORD a1, DWORD a2, DWORD a3, DWORD a4, DWORD a5)
{
	// 0x39CE2C == SimObject, 0x39D020 == ConsoleObject
	if (a4 == (ImageBase + 0x39CE2C) || a4 == (ImageBase + 0x39D020)) {
		if (a1) {
			// Printf("%x %x %x %x %x", a1, a2, a3, a4, a5);
			// Printf("%x", TestClass::getStaticClassRep());
			// Printf("%x", actual->getClassRep());

			// intercept it if it's coming right for us
			ConsoleObject* actual = (ConsoleObject*)a1;
			if (actual->getClassRep() == TestClass::getStaticClassRep())
			{
				Printf("INTERCEPTED CAST");
				return a1;
			}
		}
	}
	return PLH::FnCast(RTDynamicTramp, RTDynamicCast)(a1, a2, a3, a4, a5);
}

DWORD WINAPI Init(LPVOID args)
{
	InitScanner("Blockland.exe"); // get image base and whatnot

	if (!InitTorqueStuff()) // if we can't initialize our engine (and we should), just stop the entire thing
		return false;

	Printf("ModuleBase: %x", ImageBase);
	pSimObjectVTable = (void*)(ImageBase + 0x31A404);
	AbstractClassRep::classLinkList = (AbstractClassRep**)(ImageBase + 0x3C5D18);
	AbstractClassRep::classTable = reinterpret_cast<AbstractClassRep*****>((void*)(ImageBase + 0x3C6180));
	AbstractClassRep::NetClassCount = (U32**)(ImageBase + 0x3C6150);
	
	PLH::CapstoneDisassembler dis(PLH::Mode::x86);
	RTDynamicCast = (RTDynamicCastFn)(ImageBase + 0x2DA71B);
	ACR_Initialize = (ACR_InitializeFn)(ImageBase + 0x42510);
	ACR_InitializeDetour = new PLH::x86Detour((char*)ACR_Initialize, (char*)&h_ACRInit, &ACR_InitializeTramp, dis);
	RTDynamicDetour = new PLH::x86Detour((char*)RTDynamicCast, (char*)&h_RTDynamicCast, &RTDynamicTramp, dis);

	return RTDynamicDetour->hook() && ACR_InitializeDetour->hook();
}

//Entry point
int WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Init, NULL, 0, NULL);
	}

	return true;
}

extern "C" void __declspec(dllexport) __cdecl init() {} // we need a stub function that will kickstart the DLL