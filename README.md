# DLL-Injector
 
## Overview:

This project includes a DLL-injector written in C++.

This project is related to Windows Operation System.

To achieve the injection goal the program uses the LoadLibrary function from the Windows API.

## Method:

To inject the DLL into other processes, the injector uses the following sequence:

1. VirtualAllocEx - Allocate memory in injected process space.
2. WriteProcessMemory - Write the DLL path to the memory we have allocated.
3. CreateRemoteThread - Create a remote "LoadLibrery" thread in the injected process.

## Build:
> **Note** <br>
> Build the project based on your target process's architecture (x86 or x64).
### Visual Studio
1. Open the solution file (.sln).
2. Build the project in the selected architecture (x86 or x64).

## Usage:

The project is a CLI tool and can be used in the command line with the following operations

### Inject DLL:

Inject your DLL into the selected process by using the following command

    DLL-Injector.exe inject <PID> <DLL-Path>
        - <PID>:       Process id of a running process
        - <DLL-Path>:  Path to DLL

### Unload DLL:

Unload the selected DLL from the selected process by using the following command

    DLL-Injector.exe unload <PID> <DLL-Path>
        - <PID>:       Process id of a running process
        - <DLL-Path>:  Path to DLL
        
## Demo:
 
Demonstrating the injector functionality and usage.

The injected DLL opens a MessageBox in the injected process with the message "Process Attach/Detach" based on the selected operation.

https://user-images.githubusercontent.com/63373097/235729112-f678cff8-e1db-4f6c-a8f0-7d6268f9acc2.mp4

