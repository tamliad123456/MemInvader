#pragma once

#include <map>
#include <vector>
#include <string>
#include <Windows.h>
#include <psapi.h>
#include <iostream>
#include <tlhelp32.h>
#include <DbgHelp.h>
#include <memory>

#define MIN(x, y) (x > y ? y : x)
#define MAX(x, y) (x < y ? y : x)

#define GET(x) [&](){x}()

template<class T>
using PTR = std::shared_ptr<T>;

#define NULLPTR(T) std::shared_ptr<T>(nullptr);

class MemSnapshot;
struct Page;
#include "Process.h"

class Process;
#include "MemSnapshot.h"


