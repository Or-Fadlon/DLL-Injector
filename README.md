# DLL-Injector
 
## Overview:

This project includes a DLL-injector written in C++.
This project is related to Windows Operation System.
To achieve this goal the program uses the LoadLibrary function from the Windows API.

## Method:

To inject the DLL into other processes, the injector uses the following sequence:

1. VirtualAllocEx - Allocate memory in injected process space.
2. WriteProcessMemory - Write the DLL path to the memory we have allocated.
3. CreateRemoteThread - Create a remote "LoadLibrery" thread in the injected process.

## Usage:

The project is a CLI tool and can be used in the command line with the following operations

### Inject DLL:

Inject your DLL into the selected process by using the following command

    DLL-Injector.exe inject <PID> <DLL-Path>
        - <PID>:	   Process id of a running process
        - <DLL-Path>:  Path to DLL

### Unload DLL:

Unload the selected DLL from the selected process by using the following command

    DLL-Injector.exe hide <PID> <DLL-Path>
        - <PID>:	   Process id of a running process
        - <DLL-Path>:  Path to DLL