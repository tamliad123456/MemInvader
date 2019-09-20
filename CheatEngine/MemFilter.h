#pragma once
#include "MemInvaderInclude.h"

class MemFilter
{
	PTR<MemTable> addresses;

public:

	class Filter
	{
	public:
		enum Type
		{
			BIGGER,
			SMALLER,
			DIFFERANT,
			SAME
		};
		typedef bool(*FilterType)(char prior, char later);

		static inline bool bigger_type(char prior, char later)
		{
			return later > prior;
		}
		static inline bool smaller_type(char prior, char later)
		{
			return later < prior;
		}
		static inline bool differant_type(char prior, char later)
		{
			return later != prior;
		}
		static inline bool same_type(char prior, char later)
		{
			return later == prior;
		}

		static FilterType funcs[];
		FilterType filter;

	public:
		Filter(Type type);

		inline bool operator()(char prior, char later) { return filter(prior, later); }
	};
	

	void filter(Process& proc, Filter filter);

	void filter_new_value(Process& proc, char* value, int len);

	template<class T>
	void filter_new_value(Process& proc, T& value); //if it navigates a char array to Process::find with the right template the other "filter_new_value" arent needed

	template<int len>
	void filter_new_value(Process& proc, char (&value)[len]);

	template<int len>
	void filter_new_value(Process& proc, wchar_t (&value)[len]);

	std::vector<uint64_t> get_addresses();
};

