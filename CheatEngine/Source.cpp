#include <windows.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include "MemInvaderInclude.h"


using namespace std;


int main()
{
	auto proc = get_processes("notepad.exe")[0];

	wchar_t bla[] = L"blabla";
	

	while (1)
	{
		for (auto addr : proc.find(bla))
		{
			wchar_t w[] = L"fuckyu";
			proc.write(addr, w);
		}
	}


	/*int first = proc.take_snapshot();

	while (1)
	{
		auto first_snap = proc.get_snapshot(first);

		auto tmp = proc.take_snapshot();

		auto diff = first_snap->cmp(*proc.get_snapshot(tmp));

		if (diff.size() > 0)
		{
			for (auto pair : diff)
			{
				std::cout << pair.first << ", " << pair.second << std::endl;
			}

		}

		proc.delete_snapshot(first);
		first = tmp;
	}*/


	return 0;
}