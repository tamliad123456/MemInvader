#include <windows.h>
#include <iostream>
#include <iomanip>
#include "Process.h"



using namespace std;


int main()
{
	auto proc = get_processes("notepad.exe")[0];

	//"\x68\x00\x65\x00\x6C\x00\x6C\x00\x6F\x00"
	auto oc = proc.find((char*)"\x68\x00\x65\x00\x6C\x00\x6C\x00\x6F\x00", 11);
	proc.write(oc[0], (char*)L"fucking yes", 11*2);
	return 0;
}