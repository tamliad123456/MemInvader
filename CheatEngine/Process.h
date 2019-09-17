#pragma once

#include "MemInvaderInclude.h"

class Process
{
	HANDLE proc;
	std::string name;
	int pid;
	int parent_pid;

	PTR<std::map<int, PTR<MemSnapshot>>> snapshots;

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

	int take_snapshot();
	PTR<MemSnapshot> get_snapshot(int id);
	void delete_snapshot(int id);

};

bool is_usable(MEMORY_BASIC_INFORMATION& page_info);

std::vector<Process> get_processes(const std::string& proc_name = "");

