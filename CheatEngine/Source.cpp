#include <windows.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include "MemInvaderInclude.h"


using namespace std;


int main()
{
	auto proc = get_processes("devenv.exe")[0];

	//wchar_t bla[] = L"blabla";
	//

	//while (1)
	//{
	//	for (auto addr : proc.find(bla))
	//	{
	//		wchar_t w[] = L"fuckyu";
	//		proc.write(addr, w);
	//	}
	//}



	auto first = proc.take_snapshot();

	while (1)
	{
		auto tmp = proc.take_snapshot();
		auto bla = clock();
		auto diff = first->cmp(*tmp, Type::DIFFERANT);
		auto curr = clock();

		if (diff->size() > 0)
		{
			std::cout << curr - bla << std::endl;
		}

		first = tmp;
	}

	std::cin.get();
	return 0;
}