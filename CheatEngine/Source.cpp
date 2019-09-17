#include <windows.h>
#include <iostream>
#include <iomanip>
#include "MemInvaderInclude.h"

using namespace std;


int main()
{

	auto proc = get_processes("notepad.exe")[0];



	int id = proc.take_snapshot();
	auto snap = proc.get_snapshot(id);
	auto mem = snap->get_mem(snap->get_addresses()[0]);
	
	return 0;
}