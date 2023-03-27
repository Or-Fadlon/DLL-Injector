#include <windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <tchar.h>
#include <psapi.h>
#include <iostream>
#include <string>

bool InjectDLL(DWORD procID, const char *dllPath)
{
	// the length of the dll path
	int dllPathLen = strlen(dllPath) + 1;

	// get process handler
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, procID);
	if (hProcess == INVALID_HANDLE_VALUE)
		return false;

	// allocate memory in the process
	LPVOID exMemory = VirtualAllocEx(hProcess, NULL, dllPathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!exMemory)
	{
		CloseHandle(hProcess);
		return false;
	}

	// write the path to the dll in the process memory
	BOOL WPM = WriteProcessMemory(hProcess, exMemory, dllPath, dllPathLen, NULL);
	if (!WPM)
	{
		CloseHandle(hProcess);
		return false;
	}

	// pen a new LoadLibrary thread in the injected process
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibrary, exMemory, 0, NULL);
	if (!hThread)
	{
		VirtualFreeEx(hProcess, exMemory, NULL, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	// free and close handlers
	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(hProcess, exMemory, NULL, MEM_RELEASE);
	CloseHandle(hProcess);
	CloseHandle(hThread);
	return true;
}

// Function to get the name of a process given its ID
char *GetProcessName(DWORD processId)
{
	// Get a handle to the process
	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
	if (process == NULL)
	{
		return NULL;
	}

	// Get the process name
	char processName[MAX_PATH] = "";
	HMODULE module;
	DWORD cbNeeded;
	if (EnumProcessModules(process, &module, sizeof(module), &cbNeeded))
	{
		GetModuleBaseName(process, module, processName, sizeof(processName) / sizeof(char));
	}

	// Clean up
	CloseHandle(process);

	// Return the process name or NULL if it couldn't be retrieved
	return strlen(processName) > 0 ? processName : NULL;
}

bool FileExists(const std::string &filename)
{
	std::ifstream infile(filename.c_str());
	return infile.good();
}

int main(int argc, char *argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cout << "Usage: " << argv[0]<<" <Process-ID> <DLL-Path>" << std::endl;
			return 1;
		}
		int pid = atoi(argv[1]);
		if (GetProcessName(pid) == NULL)
		{
			std::cout << "Process ID is not of a running process" << std::endl;
			return 1;
		}
		char *dllPath = argv[2];
		if (!FileExists(dllPath))
		{
			std::cout << "Selected dll file does not exist" << std::endl;
			return 1;
		}

		bool injectionSuccess = InjectDLL((DWORD)pid, dllPath);
		if (injectionSuccess)
			std::cout << "DLL injected successfully" << std::endl;
		else
			std::cout << "DLL injection Error" << std::endl;
		return 0;
	}
	catch (const std::exception &e)
	{
		std::cout << "Exit with Error!" << std::endl;
		std::cout << e.what() << std::endl;
		return 1;
	}
}
