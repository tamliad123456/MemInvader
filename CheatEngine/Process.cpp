#include "Process.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;

/* A constructor function for process*/
Process::Process(std::string name, int pid, int parent) : name(name), pid(pid), parent_pid(parent), snapshots(0)
{
	snapshots = std::shared_ptr<std::map<int, std::shared_ptr<MemSnapshot>>>(new std::map<int, std::shared_ptr<MemSnapshot>>);
	proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

Process::Process(const Process& other)
{
	proc = NULL;
	*this = other;
}

Process& Process::operator=(const Process& other)
{
	this->name = other.name;
	this->pid = other.pid;
	this->parent_pid = other.parent_pid;
	this->snapshots = other.snapshots;

	if (proc)
	{
		CloseHandle(proc);
	}

	this->proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	return *this;
}

/* A constructor function for process*/
Process::Process(int pid) : pid(0), name(""), parent_pid(0), snapshots(0), proc(NULL)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		throw std::exception("cant create processess snapshot");
	}
	else 
	{
		pe32.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hProcessSnap, &pe32);
		if (pid == pe32.th32ProcessID)
		{
			*this = Process(pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID);
			return;
		}

		while (Process32Next(hProcessSnap, &pe32))
		{
			if (pid == pe32.th32ProcessID)
			{
				*this = Process(pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID);
				break;
			}

		}
		// clean the snapshot object
		CloseHandle(hProcessSnap);

	}
}
/* A Distructor function*/
Process::~Process()
{

	if (proc)
	{
		CloseHandle(proc);
		proc = NULL;
	}
	
}


void Process::inject_dll(const std::string& dllname)
{
	LPVOID LoadLibAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

	LPVOID dereercomp = VirtualAllocEx(proc, NULL, dllname.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(proc, dereercomp, dllname.c_str(), dllname.size(), NULL);

	HANDLE asdc = CreateRemoteThread(proc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddr, dereercomp, 0, NULL);
	WaitForSingleObject(asdc, INFINITE);

	VirtualFreeEx(proc, dereercomp, dllname.size(), MEM_RELEASE);
	CloseHandle(asdc);
}

/* A function to get a vector of pages */
std::vector<Page> Process::pages() const
{
	std::vector<Page> ret;

	MEMORY_BASIC_INFORMATION page_info;
	uint64_t addr = NULL;
	SIZE_T query_ret = sizeof(page_info);

	while (query_ret == sizeof(page_info))
	{
		query_ret = VirtualQueryEx(this->proc, (void*)addr, &page_info, sizeof(page_info));
		addr = (uint64_t)page_info.BaseAddress + page_info.RegionSize;

		if (is_usable(page_info))
		{
			ret.push_back(Page{ (uint64_t)page_info.BaseAddress, page_info.RegionSize });
		}
	} 
	
	return ret;
}

/* A function to write to process memory */
SIZE_T Process::write(uint64_t addr, char* buff, uint64_t len)
{
	SIZE_T buff_write = 0;
	WriteProcessMemory(this->proc, (void*)addr, buff, len, &buff_write);
	return buff_write;
}

/* A function to read from process memory */
SIZE_T Process::read(uint64_t addr, char* buff, uint64_t len) const
{
	SIZE_T buff_read = 0;
	ReadProcessMemory(this->proc, (void*)addr, buff, len, &buff_read);
	return buff_read;
}

/* A function to find buffer inside a page */
std::vector<uint64_t> Process::find(char* buff, int len)
{
	std::vector<uint64_t> ret;
	
	for(auto& page : this->pages())
	{
		std::string page_content(page.size, 0);
		this->read(page.base_addr, (char*)page_content.c_str(), page.size);

		string content_str = string(buff, buff + len);
		
		size_t pos = page_content.find(content_str, 0);
		while (pos != string::npos)
		{
			ret.push_back(pos + page.base_addr);
			pos = page_content.find(content_str, pos + 1);
		}
	}
	return ret;
}

int Process::take_snapshot()
{
	static int index = 0;
	
	(*snapshots)[index] = PTR<MemSnapshot>(new MemSnapshot(*this));
	return index++;
}

PTR<MemSnapshot> Process::get_snapshot(int id)
{
	auto it = snapshots->find(id);
	
	if (it != snapshots->end())
	{
		return it->second;
	}
	return nullptr;
}

void Process::delete_snapshot(int id)
{
	auto it = snapshots->find(id);

	if (it != snapshots->end())
	{
		snapshots->erase(it);
	}
	
}

/* A function that checks if page can be read and written to */
bool is_usable(MEMORY_BASIC_INFORMATION& page_info)
{
	return	page_info.State ==	 MEM_COMMIT &&
			page_info.Type ==	 MEM_PRIVATE &&  //weird
			page_info.Protect == PAGE_READWRITE;
			
}

/* A function to get all the running processes in vector */
std::vector<Process> get_processes(const std::string& proc_name)
{
	std::vector<Process> processes;
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return std::vector<Process>();
	}
	else
	{
		pe32.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hProcessSnap, &pe32);

		if(proc_name == "" || proc_name == pe32.szExeFile)
			processes.push_back(Process(pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID));

		while (Process32Next(hProcessSnap, &pe32))
		{
			if(proc_name == "" || proc_name == pe32.szExeFile)
				processes.push_back(Process(pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID));
		}
		// clean the snapshot object
		CloseHandle(hProcessSnap);

	}
	return processes;
}


