#include "MemSnapshot.h"

MemSnapshot::MemSnapshot(Process& proc)
{
	for (Page& page : proc.pages())
	{
		PTR<MemBuffer> content = PTR<MemBuffer>(new MemBuffer(page.size, 0));
		proc.read(page.base_addr, (char*)content->c_str(), page.size);

		pages[page.base_addr] = content;
	}
}

MemSnapshot::~MemSnapshot()
{
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


void MemSnapshot::cmp_buffers_different(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table)
{
	uint64_t len = MIN(prior.size(), later.size());
	uint64_t match_len = 0;

	char* a_data = (char*)prior.c_str();
	char* b_data = (char*)later.c_str();

	for (uint64_t i = 0; i < len; i++)
	{
		if (a_data[i] != b_data[i])
		{
			auto data = PTR<MemBuffer>(new MemBuffer(1, b_data[i]));
			table.emplace_back(base_addr + i, data);
		}								
	}
}

void MemSnapshot::cmp_buffers_bigger(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table)
{
	uint64_t len = MIN(prior.size(), later.size());
	uint64_t match_len = 0;

	char* a_data = (char*)prior.c_str();
	char* b_data = (char*)later.c_str();

	for (uint64_t i = 0; i < len; i++)
	{
		if (a_data[i] < b_data[i])
		{
			auto data = PTR<MemBuffer>(new MemBuffer(1, b_data[i]));
			table.emplace_back(base_addr + i, data);
		}
	}
}
void MemSnapshot::cmp_buffers_smaller(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table)
{
	uint64_t len = MIN(prior.size(), later.size());
	uint64_t match_len = 0;

	char* a_data = (char*)prior.c_str();
	char* b_data = (char*)later.c_str();

	for (uint64_t i = 0; i < len; i++)
	{
		if (a_data[i] > b_data[i])
		{
			auto data = PTR<MemBuffer>(new MemBuffer(1, b_data[i]));
			table.emplace_back(base_addr + i, data);
		}
	}
}
void MemSnapshot::cmp_buffers_same(const MemBuffer& prior, const MemBuffer& later, uint64_t base_addr, std::vector<MemValue>& table)
{
	uint64_t len = MIN(prior.size(), later.size());
	uint64_t match_len = 0;

	char* a_data = (char*)prior.c_str();
	char* b_data = (char*)later.c_str();

	for (uint64_t i = 0; i < len; i++)
	{
		if (a_data[i] == b_data[i])
		{
			auto data = PTR<MemBuffer>(new MemBuffer(1, b_data[i]));
			table.emplace_back(base_addr + i, data);
		}
	}
}