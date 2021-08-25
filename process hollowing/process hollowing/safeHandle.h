#pragma once
#include <memory>
#include <windows.h>
#include <iostream>

class SafeHandle {
public:	
	SafeHandle(HANDLE & hProcess, HANDLE & hThread);
	HANDLE& getProcess();
	HANDLE& getThread();
	~SafeHandle();
private:
	HANDLE * phProcess = nullptr;
	HANDLE * phThread = nullptr;
};