#include "Process.h"
using std::cout;
using std::endl;

process::process(std::string name, int pid, int parent) : name(name), pid(pid), parent_pid(parent)
{
	proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	this->base_addr = this->GetProcessBaseAddress();

	SYSTEM_INFO sysInfo;
	FILETIME ftime, exit, fsys, fuser;

	GetSystemInfo(&sysInfo);
	numProcessors = sysInfo.dwNumberOfProcessors;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&lastCPU, &ftime, sizeof(FILETIME));

	GetProcessTimes(proc, &ftime, &exit, &fsys, &fuser);
	memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
	memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));

	TIME_ZONE_INFORMATION zone;
	GetTimeZoneInformation(&zone);
	FileTimeToSystemTime(&ftime, &creation);
	SystemTimeToTzSpecificLocalTime(&zone, &creation, &creation);
}

process::~process()
{
}

std::vector<unsigned char> process::get_memory() const
{

	uint64_t address = base_addr;
	SIZE_T br = CHUNK_SIZE, ws = 0, pb = 0;
	this->get_memory_bytes(ws, pb);

	std::vector<unsigned char> ret(ws + pb);
	ReadProcessMemory(this->proc, (void*)base_addr, ret.data(), ws + pb, &br);
	std::cout << GetLastError() << std::endl;
	return ret;
}

uint64_t process::get_module_base_addr(std::string& modName) const
{

	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->pid);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (std::string(modEntry.szModule) == modName)
				{
					modBaseAddr = (uint64_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

uint64_t process::GetProcessBaseAddress() const
{
	uint64_t   baseAddress = 0;
	HANDLE      processHandle = this->proc;
	HMODULE* moduleArray;
	LPBYTE      moduleArrayBytes;
	DWORD       bytesRequired;

	if (processHandle)
	{
		if (EnumProcessModules(processHandle, NULL, 0, &bytesRequired))
		{
			if (bytesRequired)
			{
				moduleArrayBytes = (LPBYTE)LocalAlloc(LPTR, bytesRequired);

				if (moduleArrayBytes)
				{
					unsigned int moduleCount;

					moduleCount = bytesRequired / sizeof(HMODULE);
					moduleArray = (HMODULE*)moduleArrayBytes;

					if (EnumProcessModules(processHandle, moduleArray, bytesRequired, &bytesRequired))
					{
						baseAddress = (uint64_t)moduleArray[0];
					}

					LocalFree(moduleArrayBytes);
				}
			}
		}
	}
	return baseAddress;
}
double process::get_cpu_percent()
{
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&now, &ftime, sizeof(FILETIME));

	GetProcessTimes(proc, &ftime, &ftime, &fsys, &fuser);
	memcpy(&sys, &fsys, sizeof(FILETIME));
	memcpy(&user, &fuser, sizeof(FILETIME));
	percent = (sys.QuadPart - lastSysCPU.QuadPart) +
		(user.QuadPart - lastUserCPU.QuadPart);
	percent /= (now.QuadPart - lastCPU.QuadPart);
	percent /= numProcessors;
	lastCPU = now;
	lastUserCPU = user;
	lastSysCPU = sys;

	return percent * 100;
}

void process::get_memory_bytes(SIZE_T& working_set, SIZE_T& private_bytes) const
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(proc, (PROCESS_MEMORY_COUNTERS*)& pmc, sizeof(pmc));
	working_set = pmc.WorkingSetSize;
	private_bytes = pmc.PrivateUsage;
}

std::string process::padTo(const std::string& str, const size_t num, const char paddingChar)
{
	std::string ret = str;
	if (num > ret.size())
		ret.insert(0, num - ret.size(), paddingChar);
	return ret;
}

std::string process::get_creation_time() const
{
	return
		std::to_string(creation.wYear) + "-" +
		padTo(std::to_string(creation.wMonth), 2, '0') + "-" +
		padTo(std::to_string(creation.wDay), 2, '0') + " " +
		padTo(std::to_string(creation.wHour), 2, '0') + ":" +
		padTo(std::to_string(creation.wMinute), 2, '0') + ":" +
		padTo(std::to_string(creation.wSecond), 2, '0') + "." + std::to_string(creation.wMilliseconds);
}


std::string process::toString() const
{
	SIZE_T working_set = 0;
	SIZE_T private_bytes = 0;
	get_memory_bytes(working_set, private_bytes);
	return std::string() +
		"name: " + name +
		" pid: " + std::to_string(pid) +
		" parent: " + std::to_string(parent_pid) +
		//" cpu: " + std::to_string(get_cpu_percent()) +
		" working set: " + std::to_string(working_set) +
		" private bytes: " + std::to_string(private_bytes) +
		" creation: " + get_creation_time();
}

std::set<process> get_processes()
{
	std::set<process> processes;
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return std::set<process>();
	}
	else
	{
		pe32.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hProcessSnap, &pe32);
		processes.insert(process(pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID));

		while (Process32Next(hProcessSnap, &pe32))
		{
			processes.insert(process(pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID));
		}
		// clean the snapshot object
		CloseHandle(hProcessSnap);

	}
	return processes;
}


void process::WriteFullDump() const
{
	const DWORD Flags = MiniDumpWithFullMemory |
		MiniDumpWithFullMemoryInfo |
		MiniDumpWithHandleData |
		MiniDumpWithUnloadedModules |
		MiniDumpWithThreadInfo;

	
		auto shm = CreateFileMapping((HANDLE)0xFFFFFFFF,
			NULL,
			PAGE_READWRITE,
			0,
			1000000000,
			"dmp");
	   
	   bool bFileMappingAlreadyExists =
		   (GetLastError() == ERROR_ALREADY_EXISTS);

	   // Now map a pointer to the size tag in 
	   // the shared memory.
	   auto m_pSize = (int*)MapViewOfFile(shm,
		   FILE_MAP_ALL_ACCESS,
		   0,
		   0,
		   1000000000);

		BOOL Result = MiniDumpWriteDump(proc, pid, shm, (MINIDUMP_TYPE)Flags, NULL, NULL, NULL);

		//CloseHandle(hFile);

		/*if (!Result)
		{
			std::cerr << ("Looks like an error: MiniDumpWriteDump failed");
		}*/

	return;
}