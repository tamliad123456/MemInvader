#pragma once

#include <Windows.h>
#include <psapi.h>
#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <tlhelp32.h>
#include <DbgHelp.h>
#include <vector>


struct Page
{
	uint64_t base_addr;
	uint64_t size;
};

class Process
{
	HANDLE proc;
	std::string name;
	int pid;
	int parent_pid;


public:
	Process(std::string name, int pid, int parent);
	Process(int pid);
	~Process();

	Process(const Process& other);
	Process& operator=(const Process& other);

	inline std::string get_name() const { return name; };
	inline int get_pid() const { return pid; };
	inline int get_parent() const { return parent_pid; };
	inline bool operator>(const Process& other) const { return pid > other.pid; };
	inline bool operator<(const Process& other) const { return pid < other.pid; };
	inline bool operator==(const Process& other) const { return pid == other.pid; };
	inline HANDLE get_handle() const { return proc; };

	std::vector<Page> pages() const;

	SIZE_T write(uint64_t addr, char* buff, uint64_t len);
	SIZE_T read(uint64_t addr, char* buff, uint64_t len) const;

	std::vector<uint64_t> find(char* buff, int len);

};

bool is_usable(MEMORY_BASIC_INFORMATION& page_info);

std::vector<Process> get_processes(const std::string& proc_name = "");

