
#include "MemInvaderInclude.h"
#include <winternl.h>

#include <Accctrl.h>
#include <Aclapi.h>
#include <Sddl.h>

using namespace std;
typedef struct _THREAD_LAST_SYSCALL_INFORMATION
{
	PVOID FirstArgument;
	USHORT SystemCallNumber;

} THREAD_LAST_SYSCALL_INFORMATION, * PTHREAD_LAST_SYSCALL_INFORMATION;

PACL getACL(const std::vector<std::tuple<std::string, DWORD, ACCESS_MODE, TRUSTEE_FORM, TRUSTEE_TYPE>>& perms)
{
	PSID pEveryoneSID = NULL;
	PACL pACL = NULL;

	int len = perms.size();

	EXPLICIT_ACCESS* ea = new EXPLICIT_ACCESS[len];
	ZeroMemory(ea, len * sizeof(EXPLICIT_ACCESS));

	int i = 0;
	for (auto& [name, perm, mode, form, type] : perms)
	{
		ea[i].grfAccessPermissions = perm;
		ea[i].grfAccessMode = mode;
		ea[i].grfInheritance = NO_INHERITANCE;
		ea[i].Trustee.TrusteeForm = form;//TRUSTEE_FORM::TRUSTEE_IS_NAME;
		ea[i].Trustee.TrusteeType = type;//TRUSTEE_TYPE::TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[i].Trustee.ptstrName = (LPTSTR)name.c_str();
		i++;
	}

	SetEntriesInAcl(len, ea, NULL, &pACL);

	return pACL;
}

PSECURITY_DESCRIPTOR getPSD(PACL acl)
{
	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
	{
		return NULL;
	}

	if (!SetSecurityDescriptorDacl(pSD, TRUE, acl, FALSE))
	{
		return NULL;
	}

	return pSD;
}

int main()
{
	ChildProcess("c:\\windows\\system32\\calc.exe", "c:\\windows\\system32\\cmd.exe");
	return 0;
}