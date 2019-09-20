#pragma once
#include "MemInvaderInclude.h"

struct Page
{
	uint64_t base_addr;
	uint64_t size;
};

struct MemValue
{
	uint64_t addr;
	PTR<MemBuffer> value;

	MemValue(uint64_t addr, PTR<MemBuffer>& value) : addr(addr), value(value) {}
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

	PTR<std::vector<MemValue>> cmp(MemSnapshot& other, Type filter); //TODO: return MemFilter


	static void cmp_buffers_different(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table);
	static void cmp_buffers_bigger(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table);
	static void cmp_buffers_smaller(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table);
	static void cmp_buffers_same(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table);
};

