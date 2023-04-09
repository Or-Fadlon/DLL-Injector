#include <windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <tchar.h>
#include <psapi.h>
#include <iostream>
#include <string>

bool InjectDLL(DWORD procID, const char* dllPath)
{
	// the length of the dll path
	int dllPathLen = strlen(dllPath) + 1;

	// get process handle
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

	// free and close handles
	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(hProcess, exMemory, NULL, MEM_RELEASE);
	CloseHandle(hProcess);
	CloseHandle(hThread);
	return true;
}

bool UnloadDLL(DWORD procID, const char* dllPath)
{
	MODULEENTRY32 entry = { sizeof(MODULEENTRY32) };
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procID);

	if (Module32First(snapshot, &entry) == TRUE) {
		while (Module32Next(snapshot, &entry) == TRUE) {
			if (lstrcmpi(entry.szModule, dllPath) == 0 || lstrcmpi(entry.szExePath, dllPath) == 0) {
				// get the handle to the process
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
				if (hProcess == INVALID_HANDLE_VALUE)
					return false;
				// FreeLibrary the librery
				HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, entry.modBaseAddr, 0, NULL);
				if (!hThread) {
					CloseHandle(hProcess);
					return false;
				}
				WaitForSingleObject(hThread, INFINITE);
				// Close handles
				CloseHandle(hProcess);
				CloseHandle(hThread);
				return true;
			}
		}
	}
	// DLL not found
	return false;
}

// Function to get the name of a process given its ID
char* GetProcessName(DWORD processId)
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

bool FileExists(const std::string& filename)
{
	std::ifstream infile(filename.c_str());
	return infile.good();
}

int main(int argc, char* argv[])
{
	const char injectString[] = "inject";
	const char hideString[] = "hide";
	try
	{
		if (argc != 4)
		{
			std::cout << std::endl << "Usage: " << argv[0] << "<Operation> <PID> <DLL-Path>" << std::endl;
			std::cout << "	- <Operation>: inject / hide" << std::endl;
			std::cout << "	- <PID>:	   Process id of running process" << std::endl;
			std::cout << "	- <DLL-Path>:  Path to DLL" << std::endl;
			return 1;
		}
		char* operation = argv[1];
		if (lstrcmpi(operation, injectString) != 0 && lstrcmpi(operation, hideString) != 0)
		{
			std::cout << "Operation can be 'inject' or 'hide'" << std::endl;
			return 1;
		}
		int pid = atoi(argv[2]);
		if (GetProcessName(pid) == NULL)
		{
			std::cout << "Process ID is not of a running process" << std::endl;
			return 1;
		}
		char* dllPath = argv[3];
		if (!FileExists(dllPath))
		{
			std::cout << "Selected dll file does not exist" << std::endl;
			return 1;
		}
		if (lstrcmpi(operation, injectString) == 0) {
			bool injectionSuccess = InjectDLL((DWORD)pid, dllPath);
			if (injectionSuccess)
				std::cout << "DLL injected successfully" << std::endl;
			else
				std::cout << "DLL injection error" << std::endl;
			return !injectionSuccess;
		}
		else if (lstrcmpi(operation, hideString) == 0) {
			bool hideSuccess = UnloadDLL((DWORD)pid, dllPath);
			if (hideSuccess)
				std::cout << "DLL unloaded successfully" << std::endl;
			else
				std::cout << "DLL unload error" << std::endl;
			return !hideSuccess;
		}
		else {
			std::cout << "Operation Error..." << std::endl;
			return 1;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Exit with Error!" << std::endl;
		std::cout << e.what() << std::endl;
		return 1;
	}
}
