#pragma once
#include "MemInvaderInclude.h"


struct Page
{
	uint64_t base_addr;
	uint64_t size;
};

class MemSnapshot
{
public:
	typedef std::string MemBuffer;
	typedef std::map<uint64_t, PTR<MemBuffer>> MemTable;

private:
	MemTable pages;

public:
	MemSnapshot(Process& proc);
	~MemSnapshot();

	PTR<MemBuffer> get_mem(uint64_t addr);

	std::vector<uint64_t> get_addresses();

};

