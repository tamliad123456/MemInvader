#include "Process.h"
using std::cout;
using std::endl;

Process::Process(std::string name, int pid, int parent) : name(name), pid(pid), parent_pid(parent)
{
	proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

Process::Process(int pid) 
{
	std::set<Process> processes;
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
		processes.insert(Process(pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID));

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

Process::~Process()
{
}



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

SIZE_T Process::write(uint64_t addr, char* buff, uint64_t& len)
{
	SIZE_T buff_write = 0;
	ReadProcessMemory(this->proc, (void*)addr, buff, len, &buff_write);
	cout << GetLastError() << endl;
	return buff_write;
}

SIZE_T Process::read(uint64_t& addr, char* buff, uint64_t& len) const
{
	SIZE_T buff_read = 0;
	ReadProcessMemory(this->proc, (void*)addr, buff, len, &buff_read);
	return buff_read;
}

bool is_usable(MEMORY_BASIC_INFORMATION& page_info)
{
	return	page_info.State ==	 MEM_COMMIT &&
			//page_info.Type ==	 MEM_PRIVATE &&  //weird
			page_info.Protect == PAGE_READWRITE;
			
}

std::vector<Process> get_processes()
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
		processes.push_back(Process(pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID));

		while (Process32Next(hProcessSnap, &pe32))
		{
			processes.push_back(Process(pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID));
		}
		// clean the snapshot object
		CloseHandle(hProcessSnap);

	}
	return processes;
}