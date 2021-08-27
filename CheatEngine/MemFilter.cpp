#include "MemFilter.h"


MemFilter::MemFilter(PTR<std::vector<MemValue>> addresses) : addresses(addresses)
{
}

MemFilter::~MemFilter()
{
}
