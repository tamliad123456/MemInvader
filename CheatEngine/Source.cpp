
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
	Process a = get_processes("lsass.exe")[0];

	HANDLE hToken = a.getToken();

	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	PSID pAdminSID = NULL;

	if (!AllocateAndInitializeSid(&SIDAuthNT, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdminSID))
	{
		return 1;
	}


	PSID system,		admin,						Ausers,		admins,			users;
	//	 "S-1-5-18"		"S-1-5-21domain-500"		"S-1-5-11"	"S-1-5-32-544"	"S-1-5-32-545" ;
	PSID ntauth, lauth, everyone, wauth;
	auto s1 = ConvertStringSidToSidA("S-1-5-18", &system);
	auto s3 = ConvertStringSidToSidA("S-1-5-11", &Ausers);
	auto s4 = ConvertStringSidToSidA("S-1-5-32-544", &admins);
	auto s5 = ConvertStringSidToSidA("S-1-5-32-545", &users);

	//auto s6 = ConvertStringSidToSidA("S-1-1", &wauth);
	auto s7 = ConvertStringSidToSidA("S-1-1-0", &everyone);
	//auto s8 = ConvertStringSidToSidA("S-1-2", &lauth);
	//auto s9 = ConvertStringSidToSidA("S-1-5", &ntauth);

	auto acl = getACL({
		//{(char*)system, GENERIC_ALL | PROCESS_ALL_ACCESS, ACCESS_MODE::DENY_ACCESS, TRUSTEE_IS_SID, TRUSTEE_IS_WELL_KNOWN_GROUP},
		//{(char*)Ausers, GENERIC_ALL | PROCESS_ALL_ACCESS, ACCESS_MODE::DENY_ACCESS, TRUSTEE_IS_SID, TRUSTEE_IS_WELL_KNOWN_GROUP},
		//{(char*)admins, GENERIC_ALL , ACCESS_MODE::DENY_ACCESS, TRUSTEE_IS_SID, TRUSTEE_IS_UNKNOWN},
		{"Administrators", GENERIC_ALL | PROCESS_ALL_ACCESS , ACCESS_MODE::REVOKE_ACCESS, TRUSTEE_FORM::TRUSTEE_IS_NAME, TRUSTEE_IS_ALIAS},
		{"SYSTEM", GENERIC_ALL | PROCESS_ALL_ACCESS, ACCESS_MODE::REVOKE_ACCESS, TRUSTEE_FORM::TRUSTEE_IS_NAME, TRUSTEE_IS_ALIAS},
		{"Users", GENERIC_ALL | PROCESS_ALL_ACCESS, ACCESS_MODE::REVOKE_ACCESS, TRUSTEE_FORM::TRUSTEE_IS_NAME, TRUSTEE_IS_ALIAS},
		{"Administrators", GENERIC_ALL | PROCESS_ALL_ACCESS , ACCESS_MODE::DENY_ACCESS, TRUSTEE_FORM::TRUSTEE_IS_NAME, TRUSTEE_IS_ALIAS},
		{"SYSTEM", GENERIC_ALL | PROCESS_ALL_ACCESS, ACCESS_MODE::DENY_ACCESS, TRUSTEE_FORM::TRUSTEE_IS_NAME, TRUSTEE_IS_ALIAS},
		{"Users", GENERIC_ALL | PROCESS_ALL_ACCESS, ACCESS_MODE::DENY_ACCESS, TRUSTEE_FORM::TRUSTEE_IS_NAME, TRUSTEE_IS_ALIAS}
		//{(char*)wauth, GENERIC_ALL | PROCESS_ALL_ACCESS, ACCESS_MODE::DENY_ACCESS, TRUSTEE_IS_SID, TRUSTEE_IS_GROUP},
		//{(char*)everyone, GENERIC_ALL | PROCESS_ALL_ACCESS, ACCESS_MODE::DENY_ACCESS, TRUSTEE_IS_SID, TRUSTEE_IS_WELL_KNOWN_GROUP},
		//{(char*)lauth, GENERIC_ALL | PROCESS_ALL_ACCESS, ACCESS_MODE::DENY_ACCESS, TRUSTEE_IS_SID, TRUSTEE_IS_GROUP},
		//{(char*)ntauth, GENERIC_ALL | PROCESS_ALL_ACCESS, ACCESS_MODE::DENY_ACCESS, TRUSTEE_IS_SID, TRUSTEE_IS_GROUP}
	});

	auto pSD = getPSD(acl);

	auto bla = SetFileSecurityA("g:\\blabla.txt", DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION, pSD);
	auto err = GetLastError();
	if (SetKernelObjectSecurity(GetCurrentProcess(), DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION, pSD))
	{
		int x = 0;
		while (1) {}
	}

	
	
	//if (SetThreadToken(NULL, hToken))
	//{
	//	auto cmd = get_processes("cmd.exe")[0];
	//	TerminateProcess(cmd.get_handle(), 0);
	//}
	//
	//ChildProcess whoami("C:\\WINDOWS\\system32\\whoami.exe", token);
	//std::cout << whoami.readSTD(1024);
	
	return 0;
}