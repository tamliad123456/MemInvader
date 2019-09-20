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

	PTR<std::map<uint64_t, PTR<std::map<uint64_t, uint64_t>>>> cmp(MemSnapshot& other, MemFilter::Filter::Type filter = MemFilter::Filter::Type::DIFFERANT); //TODO: return MemFilter

	static PTR<std::map<uint64_t, uint64_t>> cmp_buffers(const MemBuffer& prior, const MemBuffer& later, MemFilter::Filter& filter);
};

