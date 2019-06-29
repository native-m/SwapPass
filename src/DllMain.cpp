#include "Main.h"

extern "C" __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		MH_Initialize(); // Init MinHook
		// Launch our Main thread
		CreateThread(nullptr, 0, Main, nullptr, 0, 0);
		break;
	case DLL_PROCESS_DETACH:
		MH_Uninitialize(); // Deinit MinHook
		break;
	}

	return TRUE;
}