#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <winsock2.h>	
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <future>

#define MIN(x, y) (x > y ? y : x)
#define MAX(x, y) (x < y ? y : x)

#define GET(x) [&](){x}()

template<class T>
using PTR = std::shared_ptr<T>;


#define NULLPTR(T) std::shared_ptr<T>(nullptr);


#include <stdio.h>
#include <Windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <DbgHelp.h>


class TcpConnection;
class Process;
class MemSnapshot;
struct Page;
struct MemValue;
class MemFilter;
enum Type : char;
class MemoryMapped;


typedef MemoryMapped MemBuffer;
typedef std::map<uint64_t, PTR<MemBuffer>> MemTable;

#include "TcpConnection.h"
#include "MemoryMapped.h"
#include "Process.h"
#include "MemFilter.h"
#include "MemSnapshot.h"


