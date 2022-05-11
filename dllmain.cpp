// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include "detours.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

typedef HANDLE(WINAPI* CreateFile_t)(
	_In_     LPCTSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	);

typedef HANDLE(WINAPI* CreateFile_tA)(
	_In_     const char*               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	);

CreateFile_t oCreateFileW;
CreateFile_tA oCreateFileA;
ofstream logfile;
const char* FlashFsCertKeysName = "\\\\.\\Xvuc\\FlashFS\\certkeys.bin";
const char* EmuFlashCertKeysName = "D:\\DevelopmentFiles\\FlashFS\\certkeys.bin";

const char* LogFileName = "D:\\emuflashlog.txt";

LPCWSTR UnicodeFlashFsCertKeysName = L"\\\\.\\Xvuc\\FlashFS\\certkeys.bin";
LPCWSTR UnicodeEmuFlashCertKeysName = L"D:\\DevelopmentFiles\\FlashFS\\certkeys.bin";

HANDLE WINAPI hkCreateFile(
	_In_     LPCTSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
)
{
	if (lpFileName == UnicodeFlashFsCertKeysName)
	{
		ofstream logfile;
		logfile.open(LogFileName);
		logfile << "System is attempting to access certkeys.bin, rerouting to EmuFlash!\n";
		logfile.close();
		return oCreateFileW(UnicodeEmuFlashCertKeysName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	}
	cout << "Normal CreateFile call, disregarding" << endl;
	return oCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HANDLE WINAPI hkCreateFileA(
	_In_     const char*               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
)
{
	if (lpFileName == FlashFsCertKeysName)
	{
		ofstream logfile;
		logfile.open(LogFileName);
		logfile << "System is attempting to access certkeys.bin, rerouting to EmuFlash!\n";
		logfile.close();
		return oCreateFileA(EmuFlashCertKeysName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	}
	cout << "Normal CreateFile call, disregarding" << endl;
	return oCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	HMODULE hKernelBase = GetModuleHandle(L"kernelbase.dll");
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		if (hKernelBase == NULL)
		{
			logfile.open(LogFileName);
			logfile << "GetModuleHandle failed\n";
			logfile.close();
			return 1;
		}
		oCreateFileW = (CreateFile_t)GetProcAddress(hKernelBase, "CreateFileW");
		oCreateFileA = (CreateFile_tA)GetProcAddress(hKernelBase, "CreateFileA");
		if (oCreateFileW == NULL)
		{
			logfile.open(LogFileName);
			logfile << "GetProcAddress CreateFileW failed\n";
			logfile.close();
			return 1;
		}
		if (oCreateFileA == NULL)
		{
			logfile.open(LogFileName);
			logfile << "GetProcAddress CreateFileA failed\n";
			logfile.close();
			return 1;
		}
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)oCreateFileW, hkCreateFile);
		DetourAttach(&(PVOID&)oCreateFileA, hkCreateFileA);
		DetourTransactionCommit();
		logfile.open(LogFileName);
		logfile << "Detouring CreateFileW\n";
		logfile << "Detouring CreateFileA\n";
		logfile.close();
		return 0;

    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)oCreateFileW, hkCreateFile);
		DetourDetach(&(PVOID&)oCreateFileA, hkCreateFileA);
		DetourTransactionCommit();
		logfile.open(LogFileName);
		logfile << "Restored original CreateFileW\n";
		logfile << "Restored original CreateFileA\n";
		logfile.close();
        break;
    }
    return TRUE;
}

