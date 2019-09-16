#include <windows.h>
#include <iostream>
#include <iomanip>
#include "Process.h"



using namespace std;


int main()
{
	while (1)
	{
		for (auto proc : get_processes("notepad.exe")) 
		{
			for (auto addr : proc.find((char*)L"תמיר", 8))
			{
				proc.write(addr, (char*)L"הומו", 8);
			}
		}
	}
	
	return 0;
}