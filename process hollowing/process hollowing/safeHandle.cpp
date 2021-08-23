#include "safeHandle.h"

SafeHanldle::SafeHanldle(HANDLE &hProcess , HANDLE &hThread) {
	phProcess = &hProcess;
	phThread = &hThread;
}

SafeHanldle::~SafeHanldle() {
	if (!CloseHandle(*phThread)) {
		std::cout << "Failed close thread\n" << GetLastError();
	}
	if (!CloseHandle(*phProcess)) {
		std::cout << "Failed close process\n" << GetLastError();
	}
}

HANDLE& SafeHanldle::getProcess() {
	return *this->phProcess;
}

HANDLE& SafeHanldle::getThread() {
	return *this->phThread;
}