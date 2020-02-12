#pragma once

#include "MemInvaderInclude.h"

class Process
{

protected:

	HANDLE proc;
	std::string name;
	int pid;
	int parent_pid;

	Process() : proc(NULL), pid(0), parent_pid(0), name("") {}

public:

	Process(std::string name, int pid, int parent);
	Process(int pid);
	virtual ~Process();

	Process(const Process& other);
	Process& operator=(const Process& other);

	inline std::string get_name() const { return name; };
	inline int get_pid() const { return pid; };
	inline int get_parent() const { return parent_pid; };
	inline bool operator>(const Process& other) const { return pid > other.pid; };
	inline bool operator<(const Process& other) const { return pid < other.pid; };
	inline bool operator==(const Process& other) const { return pid == other.pid; };
	inline HANDLE get_handle() const { return proc; };

	void inject_dll(const std::string& dllname);
	std::map<std::string, HMODULE> get_modules();
	std::vector<TcpConnection> get_tcp_connections();
	HANDLE getToken();

	std::vector<Page> pages() const;

	SIZE_T write(uint64_t addr, char* buff, uint64_t len);
	SIZE_T read(uint64_t addr, char* buff, uint64_t len) const;

	std::vector<uint64_t> find(char* buff, int len); //TODO: return MemFilter

	PTR<MemSnapshot> take_snapshot();


	template<class T>
	std::vector<uint64_t> find(T& value); //TODO: return MemFilter

	template<class T>
	void read(uint64_t addr, T& value);

	template<class T>
	void write(uint64_t addr, T& value);


	template<int len>
	std::vector<uint64_t> find(char (&value)[len]); //TODO: return MemFilter

	template<int len>
	void read(uint64_t addr, char (&value)[len]);

	template<int len>
	void write(uint64_t addr, char (&value)[len]);

	template<int len>
	std::vector<uint64_t> find(wchar_t (&value)[len]);

	template<int len>
	void read(uint64_t addr, wchar_t(&value)[len]);

	template<int len>
	void write(uint64_t addr, wchar_t(&value)[len]);

};

bool is_usable(MEMORY_BASIC_INFORMATION& page_info);

std::vector<Process> get_processes(const std::string& proc_name = "");


template<class T>
std::vector<uint64_t> Process::find(T& value)
{
	return this->find((char*)&value, sizeof(T));
}


template<class T> void Process::read(uint64_t addr, T& value)
{
	this->read(addr, (char*)& value, sizeof(T));
}


template<class T> void Process::write(uint64_t addr, T& value)
{
	this->write(addr, (char*)& value, sizeof(T));
}

template<int len>
inline std::vector<uint64_t> Process::find(wchar_t(&value)[len])
{
	return this->find((char*)& value, sizeof(value) - 2);
}

template<int len>
inline void Process::read(uint64_t addr, wchar_t(&value)[len])
{
	this->read(addr, (char*)& value, sizeof(value) - 1);
}

template<int len>
inline void Process::write(uint64_t addr, wchar_t(&value)[len])
{
	this->write(addr, (char*)& value, sizeof(value) - 1);
}

template<int len>
inline std::vector<uint64_t> Process::find(char (&value)[len])
{
	return this->find((char*)& value, sizeof(value) - 1);
}

template<int len>
inline void Process::read(uint64_t addr, char (&value)[len])
{
	this->read(addr, (char*)& value, sizeof(value) - 1);
}

template<int len>
inline void Process::write(uint64_t addr, char (&value)[len])
{
	this->write(addr, (char*)& value, sizeof(value) - 1);
}

class ChildProcess : public Process
{

	HANDLE hChildStd_IN_Rd = NULL;
	HANDLE hChildStd_IN_Wr = NULL;
	HANDLE hChildStd_OUT_Rd = NULL;
	HANDLE hChildStd_OUT_Wr = NULL;

	void setupPipes();
	void setupMetaData(const PROCESS_INFORMATION& info);

public:

	ChildProcess(const std::string& cmd);
	ChildProcess(const std::string& cmd, HANDLE token);

	~ChildProcess();

	size_t writeSTD(const std::string& data);
	std::string readSTD(size_t size);
};

