
#include "MemInvaderInclude.h"
#include <winternl.h>
using namespace std;
typedef struct _THREAD_LAST_SYSCALL_INFORMATION
{
	PVOID FirstArgument;
	USHORT SystemCallNumber;

} THREAD_LAST_SYSCALL_INFORMATION, * PTHREAD_LAST_SYSCALL_INFORMATION;



int main()
{
	auto proc = get_processes("devenv.exe")[0];

	auto modules = proc.get_modules();
	auto conenctions = proc.get_tcp_connections();

	
	return 0;
}