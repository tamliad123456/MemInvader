#pragma once
#include <memory>
#include <windows.h>
#include <iostream>

class SafeHanldle {
public:	
	SafeHanldle(HANDLE & hProcess, HANDLE & hThread);
	HANDLE& getProcess();
	HANDLE& getThread();
	~SafeHanldle();
private:
	HANDLE * phProcess = nullptr;
	HANDLE * phThread = nullptr;
};