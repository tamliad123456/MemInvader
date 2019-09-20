#include "MemFilter.h"

MemFilter::Filter::FilterType MemFilter::Filter::funcs[] = {
	&MemFilter::Filter::bigger_type,
	&MemFilter::Filter::smaller_type,
	&MemFilter::Filter::differant_type,
	&MemFilter::Filter::same_type
};

MemFilter::Filter::Filter(Type type) : filter(funcs[type])
{
}
