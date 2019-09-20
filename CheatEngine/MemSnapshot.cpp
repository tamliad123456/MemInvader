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

PTR<std::map<uint64_t, PTR<std::map<uint64_t, uint64_t>>>>  MemSnapshot::cmp(MemSnapshot& other, MemFilter::Filter::Type filter_type)
{
	auto ret = PTR<std::map<uint64_t, PTR<std::map<uint64_t, uint64_t>>>>(new std::map<uint64_t, PTR<std::map<uint64_t, uint64_t>>>);

	MemFilter::Filter filter(filter_type);
	for (auto page : pages)
	{
		auto addr = page.first;
		auto other_mem = other.get_mem(addr);

		if (other_mem)
		{
			(*ret)[addr] = cmp_buffers(*page.second, *other_mem, filter);
		}
	}

	return ret;
}

PTR<std::map<uint64_t, uint64_t>> MemSnapshot::cmp_buffers(const MemBuffer& prior, const MemBuffer& later, MemFilter::Filter& filter)
{
	auto ret = PTR<std::map<uint64_t, uint64_t>>(new std::map<uint64_t, uint64_t>);

	uint64_t match_len = 0;

	char* a_data = (char*)prior.c_str();
	char* b_data = (char*)later.c_str();

	uint64_t len = MIN(prior.size(), later.size());
	for (uint64_t i = 0; i < len; i++)
	{
		if (filter(a_data[i], b_data[i]))
		{
			match_len++;
		}
		else if (match_len > 0)
		{
			(*ret)[i - match_len] = match_len;
			match_len = 0;
		}
	}

	return ret;
}
