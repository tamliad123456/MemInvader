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

PTR<MemSnapshot::MemBuffer> MemSnapshot::get_mem(uint64_t addr)
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

std::map<uint64_t, uint64_t>  MemSnapshot::cmp(MemSnapshot& other)
{
	auto ret = std::map<uint64_t, uint64_t>();

	for(auto page : pages)
	{ 
		auto addr = page.first;
		auto other_mem = other.get_mem(addr);

		if (other_mem)
		{
			for (auto pair : cmp_buffers(*page.second, *other_mem))
			{
				ret[pair.first + page.first] = pair.second;
			}
		}
	}

	return ret;
}

std::map<uint64_t, uint64_t> MemSnapshot::cmp_buffers(const MemBuffer& a, const MemBuffer& b)
{
	auto ret = std::map<uint64_t, uint64_t>();

	uint64_t match_len = 0;

	for (uint64_t i = 0; i < MIN(a.size(), b.size()); i++)
	{
		if (a[i] != b[i])
		{
			match_len++;
		}
		else if (match_len > 0)
		{
			ret[i - match_len] = match_len;
			match_len = 0;
		}
	}

	return ret;
}
