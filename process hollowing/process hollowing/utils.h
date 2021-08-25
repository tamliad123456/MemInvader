#pragma once
#include <windows.h>
#include <iostream>
#include <optional>
#include "safeHandle.h"

#define SIZE_OF_SHELLCODE 25
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) == 0)

typedef NTSTATUS(WINAPI* pNtUnmapViewOfSection)(HANDLE, PVOID);

typedef struct BASE_RELOCATION_BLOCK {
	DWORD PageAddress;
	DWORD BlockSize;
} BASE_RELOCATION_BLOCK, * PBASE_RELOCATION_BLOCK;

typedef struct BASE_RELOCATION_ENTRY {
	USHORT Offset : 12;
	USHORT Type : 4;
} BASE_RELOCATION_ENTRY, * PBASE_RELOCATION_ENTRY;

typedef struct PE_IMAGE {
	LPVOID address;
	PIMAGE_NT_HEADERS64 pNTHeaders;
} PE_IMAGE;


namespace Utils {
	inline DWORD64 Error(const std::string& errMsg);
	BOOL getContext(HANDLE hThread, CONTEXT &ctx);
	std::optional<LPVOID> readProcessMemory(HANDLE hProcess, DWORD64 pebAddr);
	DWORD64 unmapProcess(HMODULE dll, HANDLE hProcess, LPVOID imageBaseAddr);
	PBYTE readFile(LPSTR filename);
	BOOL mapFile(std::unique_ptr<SafeHandle> &safeHandle, PBYTE buffer, LPVOID imageBaseAddr, PE_IMAGE& pe_image);
	BOOL WriteImageBase(std::unique_ptr<SafeHandle>& safeHandle, CONTEXT& ctx, PE_IMAGE& pe_image);
	BOOL setContextAndResumeThread(std::unique_ptr<SafeHandle>& safeHandle, CONTEXT& ctx);
}