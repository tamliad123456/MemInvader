#include "Process.h"
#include <codecvt>

using std::cout;
using std::endl;
using std::vector;
using std::string;


/* A constructor function for process*/
Process::Process(std::string name, int pid, int parent) : name(name), pid(pid), parent_pid(parent)
{
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

	if (proc)
	{
		CloseHandle(proc);
	}

	this->proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	return *this;
}

/* A constructor function for process*/
Process::Process(int pid) : pid(0), name(""), parent_pid(0), proc(NULL)
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
	MemSnapshot::DeleteDirectory(std::to_string(pid).c_str());
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

std::map<std::string, HMODULE> Process::get_modules()
{
	HMODULE hMods[1024];
	DWORD cbNeeded;

	std::map<std::string, HMODULE> ret;

	if (EnumProcessModules(proc, hMods, sizeof(hMods), &cbNeeded))
	{
		for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];

			// Get the full path to the module's file.

			if (GetModuleFileNameEx(proc, hMods[i], szModName,
				sizeof(szModName) / sizeof(TCHAR)))
			{
				// Print the module name and handle value.

				ret[szModName] = hMods[i];
			}
		}
	}

	return ret;
}

std::vector<TcpConnection> Process::get_tcp_connections()
{

	std::vector<TcpConnection> ret;

	PMIB_TCPTABLE2 pTcpTable;
	ULONG ulSize = 0;
	DWORD dwRetVal = 0;


	pTcpTable = (MIB_TCPTABLE2*)malloc(sizeof(MIB_TCPTABLE2));
	if (pTcpTable == NULL) {
		return ret;
	}

	ulSize = sizeof(MIB_TCPTABLE);
	// Make an initial call to GetTcpTable2 to
	// get the necessary size into the ulSize variable
	if ((dwRetVal = GetTcpTable2(pTcpTable, &ulSize, TRUE)) ==
		ERROR_INSUFFICIENT_BUFFER) {
		delete pTcpTable;
		pTcpTable = (MIB_TCPTABLE2*)malloc(ulSize);
		if (pTcpTable == NULL) {
			return ret;
		}
	}
	// Make a second call to GetTcpTable2 to get
	// the actual data we require
	if ((dwRetVal = GetTcpTable2(pTcpTable, &ulSize, TRUE)) == NO_ERROR) {
		//pTcpTable->dwNumEntries
		//table[i].dwState

		std::vector<MIB_TCPROW2> by_proc;
		std::copy_if(pTcpTable->table, pTcpTable->table + pTcpTable->dwNumEntries, std::back_inserter(by_proc), [this](MIB_TCPROW2& row) {
			return row.dwOwningPid == this->get_pid();
			});

		for (auto& row : by_proc) {
			ret.emplace_back(row);
		}
	}
	else {
		delete pTcpTable;
		return ret;
	}

	if (pTcpTable != NULL) {
		delete pTcpTable;
		pTcpTable = NULL;
	}

	return ret;
}

HANDLE Process::getToken()
{
	HANDLE token = NULL;
	if (!OpenProcessToken(proc, TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE | TOKEN_QUERY, &token))
	{
		return NULL;
	}

	HANDLE dup = NULL;
	TOKEN_TYPE tokenType = TokenPrimary;
	if (!DuplicateTokenEx(token, MAXIMUM_ALLOWED, NULL, _SECURITY_IMPERSONATION_LEVEL::SecurityImpersonation, tokenType, &dup))
	{
		return NULL;
	}
	return dup;
}

/* A function to get a vector of pages */
std::vector<Page> Process::pages() const
{
	std::vector<Page> ret;
	ret.reserve(150);

	MEMORY_BASIC_INFORMATION page_info;
	uint64_t addr = NULL;
	SIZE_T query_ret = sizeof(page_info);

	while (query_ret == sizeof(page_info))
	{
		query_ret = VirtualQueryEx(this->proc, (void*)addr, &page_info, sizeof(page_info));
		addr = (uint64_t)page_info.BaseAddress + page_info.RegionSize;

		if (is_usable(page_info))
		{
			ret.emplace_back((uint64_t)page_info.BaseAddress, page_info.RegionSize);
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
	ret.reserve(10);
	
	for(auto& page : this->pages())
	{
		std::string page_content(page.size, 0);
		this->read(page.base_addr, (char*)page_content.c_str(), page.size);

		string content_str = string(buff, buff + len);
		
		size_t pos = page_content.find(content_str, 0);
		while (pos != string::npos)
		{
			ret.emplace_back(pos + page.base_addr);
			pos = page_content.find(content_str, pos + 1);
		}
	}
	ret.shrink_to_fit();
	return ret;
}

PTR<MemSnapshot> Process::take_snapshot()
{
	static int index = 0;
	
	return std::make_shared<MemSnapshot>(*this, std::to_string(++index));
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
	processes.reserve(50);
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
	processes.shrink_to_fit();
	return processes;
}

void ChildProcess::setupPipes()
{
	SECURITY_ATTRIBUTES saAttr;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0))
		throw std::exception("blabla");

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		throw std::exception("blabla");

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
		throw std::exception("blabla");

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		throw std::exception("blabla");
}

void ChildProcess::setupMetaData(const PROCESS_INFORMATION& info)
{
	this->proc = info.hProcess;
	this->pid = info.dwProcessId;
	this->parent_pid = GetCurrentProcessId();

	TCHAR Buffer[MAX_PATH] = { 0 };
	if (GetModuleFileNameEx(this->proc, 0, Buffer, MAX_PATH))
	{
		this->name = Buffer;
	}
}

ChildProcess::ChildProcess(const std::string& cmd)
{
	STARTUPINFO info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;

	setupPipes();

	info.cb = sizeof(STARTUPINFO);
	info.hStdError = hChildStd_OUT_Wr;
	info.hStdOutput = hChildStd_OUT_Wr;
	info.hStdInput = hChildStd_IN_Rd;
	info.dwFlags |= STARTF_USESTDHANDLES;

	if (!CreateProcessA(NULL, (char*)cmd.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
	{
		throw std::exception("blabla");
	}

	setupMetaData(processInfo);
}

ChildProcess::ChildProcess(const std::string& cmd, HANDLE token)
{
	STARTUPINFOW info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;

	setupPipes();

	info.cb = sizeof(STARTUPINFO);
	info.hStdError = hChildStd_OUT_Wr;
	info.hStdOutput = hChildStd_OUT_Wr;
	info.hStdInput = hChildStd_IN_Rd;
	info.dwFlags |= STARTF_USESTDHANDLES;

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	if (!CreateProcessWithTokenW(token, LOGON_NETCREDENTIALS_ONLY, converter.from_bytes(cmd).c_str(), NULL, CREATE_NEW_CONSOLE, NULL, NULL, &info, &processInfo))
	{
		DWORD lastError;
		lastError = GetLastError();

		throw std::exception("blabla");
	}

}

ChildProcess::~ChildProcess()
{
	WaitForSingleObject(this->proc, INFINITE);
	CloseHandle(this->proc);
	this->proc = NULL;

	CloseHandle(this->hChildStd_IN_Rd);
	this->hChildStd_IN_Rd = NULL;

	CloseHandle(this->hChildStd_IN_Wr);
	this->hChildStd_IN_Wr = NULL;

	CloseHandle(this->hChildStd_OUT_Rd);
	this->hChildStd_OUT_Rd = NULL;

	CloseHandle(this->hChildStd_OUT_Wr);
	this->hChildStd_OUT_Wr = NULL;
}

size_t ChildProcess::write(const std::string& data)
{
	DWORD ret = 0;
	WriteFile(hChildStd_IN_Wr, data.c_str(), data.size(), &ret, NULL);
	return ret;
}

std::string ChildProcess::read(size_t size)
{
	DWORD read = 0;
	std::string ret("", size);

	ReadFile(hChildStd_OUT_Rd, (char*)ret.c_str(), size, &read, NULL);
	ret.resize(read);

	return ret;
}
