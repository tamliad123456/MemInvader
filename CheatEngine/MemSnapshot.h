#pragma once
#include "MemInvaderInclude.h"


struct Page
{
	uint64_t base_addr;
	uint64_t size;
};

class MemSnapshot
{

private:
	MemTable pages;

public:
	MemSnapshot(Process& proc);
	~MemSnapshot();

	PTR<MemBuffer> get_mem(uint64_t addr);

	std::vector<uint64_t> get_addresses();

	std::map<uint64_t, uint64_t>  cmp(MemSnapshot& other); //TODO: return MemFilter

	static std::map<uint64_t, uint64_t> cmp_buffers(const MemBuffer& a, const MemBuffer& b); //TODO: return MemTable, with the data instead of the length (may hinder performance)
};

