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
	return PTR<MemBuffer>(nullptr);
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
