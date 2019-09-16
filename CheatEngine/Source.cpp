#include <windows.h>
#include <iostream>
#include <iomanip>
#include "Process.h"
#include <vector>


using namespace std;


int main()
{
	uint64_t  address = 0;
	uint64_t value = 0;
	DWORD pid;
	HANDLE phandle = NULL;

	process* notepad = NULL;
	char char_to_search = 'j';

	for (auto& proc : get_processes())
	{
		if (proc.get_name() == "notepad.exe")
		{
			vector<unsigned char> old_dump;
			vector<unsigned char> diff;
			while (TRUE)
			{
				proc.WriteFullDump();
				/*if (old_dump.size() > 0)
				{
					int counter = 0;
					cout << string(dump.begin(), dump.end()) << endl;
					cout << dump.size() << endl;
				}
				old_dump = dump;*/

				//cout << string(dump.begin(), dump.end()) << endl;
				break;
			}
			break;
		}
	}

	return 0;
}