#include <windows.h>
#include <iostream>
#include <iomanip>
#include "Process.h"
#include <vector>


using namespace std;


int main()
{

	for (Process& proc : get_processes())
	{
		if (proc.get_name() == "notepad.exe")
		{
			vector<unsigned char> old_dump;
			vector<unsigned char> diff;
			while (TRUE)
			{
				auto pages = proc.pages();
				for (Page page : pages)
				{
					std::vector<char> data(page.size);
					proc.read(page.base_addr, data.data(), page.size);
					//std::cout << string(data.begin(), data.end());

					string bla = string(data.begin(), data.end());
					string r("\x68\x00\x65\x00\x6C\x00\x6C\x00\x6F\x00", 9);
					auto index = bla.find(r);
					if (index != std::string::npos)
					{
						std::cout << "blabla";
						uint64_t tmp = 12;
						proc.write(page.base_addr + index, (char*)"fucking yes", tmp);
					}
				}
				break;
			}
			break;
		}
	}

	return 0;
}