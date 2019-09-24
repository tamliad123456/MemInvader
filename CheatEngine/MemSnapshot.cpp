#include "MemSnapshot.h"

MemSnapshot::MemSnapshot(Process& proc, const std::string& name)
{
	dir = std::to_string(proc.get_pid());

	if (CreateDirectory(dir.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		dir += "\\" + name;
		if (CreateDirectory(dir.c_str(), NULL) ||
			ERROR_ALREADY_EXISTS == GetLastError())
		{
			for (Page& page : proc.pages())
			{
				auto page_path = dir + "\\" + std::to_string(page.base_addr);

				PTR<MemoryMapped> file = create_disk_buffer(page_path, page.size);
				proc.read(page.base_addr, (char*)file->getData(), page.size);

				pages[page.base_addr] = file;
			}
		}
	}	
}

MemSnapshot::~MemSnapshot()
{
	pages.clear();
	RemoveDirectory(dir.c_str());
}

PTR<MemBuffer> MemSnapshot::get_mem(uint64_t addr)
{
	auto it = pages.find(addr);
	if (it != pages.end())
	{
		return it->second;
	}
	return NULLPTR(MemBuffer);
}

std::vector<uint64_t> MemSnapshot::get_addresses()
{
	//i was bored...;

	return GET
	(
		int i = 0;
	std::vector<uint64_t> ret(pages.size());
	for (auto page : pages)
	{
		ret[i] = page.first;
		i++;
	}
	return ret;
	);
}


PTR<std::vector<MemValue>>  MemSnapshot::cmp(MemSnapshot& other, Type filter_type)
{
	PTR<std::vector<MemValue>> ret = PTR<std::vector<MemValue>>(new std::vector<MemValue>());

	void (*comp)(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table) = nullptr;

	switch (filter_type)
	{
	case BIGGER:
		comp = cmp_buffers_bigger;
		break;
	case SMALLER:
		comp = cmp_buffers_smaller;
		break;
	case DIFFERANT:
		comp = cmp_buffers_different;
		break;
	case SAME:
		comp = cmp_buffers_same;
		break;
	default:
		return nullptr;
	}



	for (auto page : pages)
	{
		auto addr = page.first;
		auto other_mem = other.get_mem(addr);

		if (other_mem)
		{
			comp(*page.second, *other_mem, addr, *ret);
		}
	}

	return ret;
}

PTR<MemoryMapped> MemSnapshot::create_disk_buffer(const std::string& name, uint64_t size)
{
	FILE* fp;

	uint64_t chunk = INT_MAX;

	fp = fopen(name.c_str(), "wb");
	while (size > chunk)
	{
		fseek(fp, chunk, SEEK_CUR);
		size -= chunk;
	}
	fseek(fp, MAX(size - 1, 0), SEEK_CUR);

	fputc(0, fp);
	fclose(fp);

	return std::shared_ptr<MemoryMapped>(new MemoryMapped(name));
}


void MemSnapshot::cmp_buffers_different(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table)
{
	uint64_t len = MIN(prior.size(), later.size());
	uint64_t match_len = 0;

	char* a_data = (char*)prior.getData();
	char* b_data = (char*)later.getData();

	for (uint64_t i = 0; i < len; i++)
	{
		if (a_data[i] != b_data[i])
		{
			table.emplace_back(base_addr + i, b_data[i]);
		}								
	}
}

void MemSnapshot::cmp_buffers_bigger(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table)
{
	uint64_t len = MIN(prior.size(), later.size());
	uint64_t match_len = 0;

	char* a_data = (char*)prior.getData();
	char* b_data = (char*)later.getData();

	for (uint64_t i = 0; i < len; i++)
	{
		if (a_data[i] < b_data[i])
		{
			table.emplace_back(base_addr + i, b_data[i]);
		}								
	}
}
void MemSnapshot::cmp_buffers_smaller(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table)
{
	uint64_t len = MIN(prior.size(), later.size());
	uint64_t match_len = 0;

	char* a_data = (char*)prior.getData();
	char* b_data = (char*)later.getData();

	for (uint64_t i = 0; i < len; i++)
	{
		if (a_data[i] > b_data[i])
		{
			table.emplace_back(base_addr + i, b_data[i]);
		}
	}
}
void MemSnapshot::cmp_buffers_same(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table)
{
	uint64_t len = MIN(prior.size(), later.size());
	uint64_t match_len = 0;

	char* a_data = (char*)prior.getData();
	char* b_data = (char*)later.getData();

	for (uint64_t i = 0; i < len; i++)
	{
		if (a_data[i] == b_data[i])
		{
			table.emplace_back(base_addr + i, b_data[i]);
		}
	}
}

bool MemSnapshot::IsDots(const char* str) {
	if (strcmp(str, ".") && strcmp(str, "..")) return FALSE;
	return TRUE;
}

bool MemSnapshot::DeleteDirectory(const char* sPath) {
	HANDLE hFind;  // file handle
	WIN32_FIND_DATA FindFileData;

	TCHAR DirPath[MAX_PATH];
	TCHAR FileName[MAX_PATH];

	strcpy(DirPath, sPath);
	strcat(DirPath, "\\*");    // searching all files
	strcpy(FileName, sPath);
	strcat(FileName, "\\");

	hFind = FindFirstFile(DirPath, &FindFileData); // find the first file
	if (hFind == INVALID_HANDLE_VALUE) return FALSE;
	strcpy(DirPath, FileName);

	bool bSearch = true;
	while (bSearch) { // until we finds an entry
		if (FindNextFile(hFind, &FindFileData)) {
			if (IsDots(FindFileData.cFileName)) continue;
			strcat(FileName, FindFileData.cFileName);
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

				// we have found a directory, recurse
				if (!DeleteDirectory(FileName)) {
					FindClose(hFind);
					return FALSE; // directory couldn't be deleted
				}
				RemoveDirectory(FileName); // remove the empty directory
				strcpy(FileName, DirPath);
			}
			else {
				if (!DeleteFile(FileName)) {  // delete the file
					FindClose(hFind);
					return FALSE;
				}
				strcpy(FileName, DirPath);
			}
		}
		else {
			if (GetLastError() == ERROR_NO_MORE_FILES) // no more files there
				bSearch = false;
			else {
				// some error occured, close the handle and return FALSE
				FindClose(hFind);
				return FALSE;
			}

		}

	}
	FindClose(hFind);  // closing file handle

	return RemoveDirectory(sPath); // remove the empty directory

}