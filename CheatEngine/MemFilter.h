#pragma once
#include "MemInvaderInclude.h"

enum Type : char
{
	BIGGER,
	SMALLER,
	DIFFERANT,
	SAME
};


class MemFilter
{
	PTR<std::vector<MemValue>> addresses;

public:

	MemFilter(PTR<std::vector<MemValue>>);
	~MemFilter();

	void filter(Process& proc, Type filter);

	void filter_new_value(Process& proc, char* value, int len);

	template<class T>
	void filter_new_value(Process& proc, T& value); //if it navigates a char array to Process::find with the right template the other "filter_new_value" arent needed

	template<int len>
	void filter_new_value(Process& proc, char (&value)[len]);

	template<int len>
	void filter_new_value(Process& proc, wchar_t (&value)[len]);

	std::vector<uint64_t> get_addresses();
};

