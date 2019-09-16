#pragma once

#include <Windows.h>
#include <psapi.h>
#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <tlhelp32.h>
#include <DbgHelp.h>
//#include <Lmcons.h>
//#include <limits>

#define CHUNK_SIZE (1024 * 128)

class process
{
	HANDLE proc;
	std::string name;
	int pid;
	int parent_pid;

	ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
	int numProcessors;
	SYSTEMTIME creation;
	uint64_t base_addr;


public:
	process(std::string name, int pid, int parent);
	~process();

	inline std::string get_name() const { return name; };
	inline int get_pid() const { return pid; };
	inline int get_parent() const { return parent_pid; };
	inline bool operator>(const process& other) const { return pid > other.pid; };
	inline bool operator<(const process& other) const { return pid < other.pid; };
	inline bool operator==(const process& other) const { return pid == other.pid; };
	inline HANDLE get_handle() const { return proc; };

	double get_cpu_percent();
	void get_memory_bytes(SIZE_T& working_set, SIZE_T& private_bytes) const;
	std::string get_creation_time() const;

	uint64_t get_module_base_addr(std::string& modName) const;
	uint64_t GetProcessBaseAddress() const;

	std::vector<unsigned char> get_memory() const;

	std::string toString() const;

	static std::string padTo(const std::string& str, const size_t num, const char paddingChar = ' ');
	void WriteFullDump() const;

};

std::set<process> get_processes();

