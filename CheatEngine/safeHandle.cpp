#include "safeHandle.h"

SafeHandle::SafeHandle(HANDLE &hProcess , HANDLE &hThread) {
	phProcess = &hProcess;
	phThread = &hThread;
}

SafeHandle::~SafeHandle() {
	if (!CloseHandle(*phThread)) {
		std::cout << "Failed close thread\n" << GetLastError();
	}
	if (!CloseHandle(*phProcess)) {
		std::cout << "Failed close process\n" << GetLastError();
	}
}

HANDLE& SafeHandle::getProcess() {
	return *this->phProcess;
}

HANDLE& SafeHandle::getThread() {
	return *this->phThread;
}