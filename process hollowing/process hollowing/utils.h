#pragma once
#include <windows.h>
#include <iostream>
#include <optional>

#define SIZE_OF_SHELLCODE 25
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) == 0)

typedef NTSTATUS(WINAPI* pNtUnmapViewOfSection)(HANDLE, PVOID);

namespace Utils {
	inline DWORD64 Error(const std::string& errMsg);
	BOOL getContext(HANDLE hThread, CONTEXT &ctx);
	std::optional<LPVOID> readProcessMemory(HANDLE hProcess, DWORD64 pebAddr);
	DWORD64 unmapProcess(HMODULE dll, HANDLE hProcess, LPVOID imageBaseAddr);
}