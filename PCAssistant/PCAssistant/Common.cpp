#include "stdafx.h"
#include "Common.h"

typedef LONG NTSTATUS;
extern CWnd* g_wParent;
HANDLE g_hDevice = NULL;

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS       ((NTSTATUS)0x00000000L)
#endif
#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#endif

BOOL Is64BitWindows()
{
#if defined(_WIN64)
	return TRUE;  // 64λ����ֻ��Win64ϵͳ������
#elif defined(_WIN32)
	// 32λ������32/64λϵͳ�����С�
	// ���Ա����ж�
	BOOL f64 = FALSE;

	return IsWow64Process(GetCurrentProcess(), &f64) && f64;
#else
	return FALSE; // Win64��֧��16λϵͳ
#endif
}


//װ��NT��������
BOOL LoadNTDriver(WCHAR* lpszDriverName, WCHAR* lpszDriverPath)
{

	WCHAR szDriverImagePath[256];

	//�õ�����������·��
	GetFullPathNameW(lpszDriverPath, 256, szDriverImagePath, NULL);

	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;//SCM�������ľ��
	SC_HANDLE hServiceDDK = NULL;//NT��������ķ�����

	//�򿪷�����ƹ�����
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hServiceMgr == NULL)
	{
		//OpenSCManagerʧ��
		printf("OpenSCManager() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		////OpenSCManager�ɹ�
		printf("OpenSCManager() ok ! \n");
	}

	//������������Ӧ�ķ���
	hServiceDDK = CreateServiceW(hServiceMgr,
		lpszDriverName,        // �����������ע����е�����
		lpszDriverName,        // ע������������ DisplayName ֵ
		SERVICE_ALL_ACCESS,    // ������������ķ���Ȩ��
		SERVICE_KERNEL_DRIVER, // ��ʾ���صķ�������������
		SERVICE_DEMAND_START,  // ע������������ Start ֵ
		SERVICE_ERROR_IGNORE,  // ע������������ ErrorControl ֵ
		szDriverImagePath,     // ע������������ ImagePath ֵ
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	DWORD dwRtn;

	//�жϷ����Ƿ�ʧ��
	if (hServiceDDK == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			//��������ԭ�򴴽�����ʧ��
			printf("CrateService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			//���񴴽�ʧ�ܣ������ڷ����Ѿ�������
			printf("CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n");
		}

		// ���������Ѿ����أ�ֻ��Ҫ��
		hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);

		if (hServiceDDK == NULL)
		{
			//����򿪷���Ҳʧ�ܣ�����ζ����
			dwRtn = GetLastError();
			printf("OpenService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;

		}
		else
		{
			printf("OpenService() ok ! \n");
		}
	}
	else
	{
		printf("CrateService() ok ! \n");
	}

	//�����������
	bRet = StartService(hServiceDDK, NULL, NULL);

	if (!bRet)
	{
		DWORD dwRtn = GetLastError();

		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			printf("StartService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			if (dwRtn == ERROR_IO_PENDING)
			{
				//�豸����ס
				printf("StartService() Faild ERROR_IO_PENDING ! \n");
				bRet = FALSE;
				goto BeforeLeave;
			}
			else
			{
				//�����Ѿ�����
				printf("StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \n");
				bRet = TRUE;
				goto BeforeLeave;
			}
		}
	}

	bRet = TRUE;

	//�뿪ǰ�رվ��
BeforeLeave:

	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}

	return bRet;

}


//ж����������
BOOL UnloadNTDriver(WCHAR* szSvrName)
{

	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;//SCM�������ľ��
	SC_HANDLE hServiceDDK = NULL;//NT��������ķ�����

	SERVICE_STATUS SvrSta;

	//��SCM������
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hServiceMgr == NULL)
	{

		//����SCM������ʧ��
		printf("OpenSCManager() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;

	}
	else
	{

		//����SCM������ʧ�ܳɹ�
		printf("OpenSCManager() ok ! \n");
	}

	//����������Ӧ�ķ���
	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);

	if (hServiceDDK == NULL)
	{
		//����������Ӧ�ķ���ʧ��
		printf("OpenService() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		printf("OpenService() ok ! \n");
	}

	//ֹͣ�����������ֹͣʧ�ܣ�ֻ�������������ܣ��ٶ�̬���ء�
	if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta))
	{
		printf("ControlService() Faild %d !\n", GetLastError());
	}
	else
	{
		//����������Ӧ��ʧ��
		printf("ControlService() ok !\n");
	}

	//��̬ж����������
	if (!DeleteService(hServiceDDK))
	{
		//ж��ʧ��
		printf("DeleteSrevice() Faild %d !\n", GetLastError());
	}
	else
	{
		//ж�سɹ�
		printf("DelServer:eleteSrevice() ok !\n");
	}

	bRet = TRUE;

BeforeLeave:

	//�뿪ǰ�رմ򿪵ľ��
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}

	return bRet;

}

VOID SendStatusDetail(LPCWSTR szBuffer)
{
	::SendMessageW(g_wParent->m_hWnd, MESSAGE_STATUSDETAIL, NULL, (LPARAM)szBuffer);
}

VOID SendStatusTip(LPCWSTR szBuffer)
{
	::SendMessageW(g_wParent->m_hWnd, MESSAGE_STATUSTIP, NULL, (LPARAM)szBuffer);
}


WIN_VERSION  GetWindowsVersion()
{
	OSVERSIONINFOEX	OsVerInfoEx;
	OsVerInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *)&OsVerInfoEx); // ע��ת������

	switch (OsVerInfoEx.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
	{
		if (OsVerInfoEx.dwMajorVersion <= 4)
		{
			return WindowsNT;
		}
		if (OsVerInfoEx.dwMajorVersion == 5 && OsVerInfoEx.dwMinorVersion == 0)
		{
			return Windows2000;
		}

		if (OsVerInfoEx.dwMajorVersion == 5 && OsVerInfoEx.dwMinorVersion == 1)
		{
			return WindowsXP;
		}
		if (OsVerInfoEx.dwMajorVersion == 5 && OsVerInfoEx.dwMinorVersion == 2)
		{
			return Windows2003;
		}
		if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 0)
		{
			return WindowsVista;
		}

		if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 1)
		{
			return Windows7;
		}
		if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 2)
		{
			return Windows8;
		}
		if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 3)
		{
			return Windows8_1;
		}
		if (OsVerInfoEx.dwMajorVersion == 10 && OsVerInfoEx.dwMinorVersion == 0)
		{
			return Windows10;
		}

		break;
	}

	default:
	{
		return WinUnknown;
	}
	}

	return WinUnknown;
}


CString TrimPath(WCHAR * wzPath)
{
	CString strPath;

	if (wzPath[1] == ':' && wzPath[2] == '\\')
	{
		strPath = wzPath;
	}
	else if (wcslen(wzPath) > wcslen(L"\\SystemRoot\\") &&
		!_wcsnicmp(wzPath, L"\\SystemRoot\\", wcslen(L"\\SystemRoot\\")))
	{
		WCHAR szSystemDir[MAX_PATH] = { 0 };
		GetWindowsDirectory(szSystemDir, MAX_PATH);
		strPath.Format(L"%s\\%s", szSystemDir, wzPath + wcslen(L"\\SystemRoot\\"));
	}
	else if (wcslen(wzPath) > wcslen(L"system32\\") &&
		!_wcsnicmp(wzPath, L"system32\\", wcslen(L"system32\\")))
	{
		WCHAR szSystemDir[MAX_PATH] = { 0 };
		GetWindowsDirectory(szSystemDir, MAX_PATH);
		strPath.Format(L"%s\\%s", szSystemDir, wzPath/* + wcslen(L"system32\\")*/);
	}
	else if (wcslen(wzPath) > wcslen(L"\\??\\") &&
		!_wcsnicmp(wzPath, L"\\??\\", wcslen(L"\\??\\")))
	{
		strPath = wzPath + wcslen(L"\\??\\");
	}
	else if (wcslen(wzPath) > wcslen(L"%ProgramFiles%") &&
		!_wcsnicmp(wzPath, L"%ProgramFiles%", wcslen(L"%ProgramFiles%")))
	{
		WCHAR szSystemDir[MAX_PATH] = { 0 };
		if (GetWindowsDirectory(szSystemDir, MAX_PATH) != 0)
		{
			CString szTemp = szSystemDir;
			szTemp = szTemp.Left(szTemp.Find('\\'));
			szTemp += L"\\Program Files";
			szTemp += wzPath + wcslen(L"%ProgramFiles%");
			strPath = szTemp;
		}
	}
	else
	{
		strPath = wzPath;
	}

	strPath = GetLongPath(strPath);

	return strPath;
}

CString GetLongPath(CString szPath)
{
	CString strPath;

	if (szPath.Find(L'~') != -1)
	{
		WCHAR szLongPath[MAX_PATH] = { 0 };
		DWORD nRet = GetLongPathName(szPath, szLongPath, MAX_PATH);
		if (nRet >= MAX_PATH || nRet == 0)
		{
			strPath = szPath;
		}
		else
		{
			strPath = szLongPath;
		}
	}
	else
	{
		strPath = szPath;
	}

	return strPath;
}


ULONG_PTR GetKernelBase(char* szNtosName)
{
	typedef long(__stdcall *pfnZwQuerySystemInformation)
		(
			IN ULONG SystemInformationClass,
			IN PVOID SystemInformation,
			IN ULONG SystemInformationLength,
			IN PULONG ReturnLength OPTIONAL
			);
	typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY64
	{
		ULONG Unknow1;
		ULONG Unknow2;
		ULONG Unknow3;
		ULONG Unknow4;
		PVOID Base;
		ULONG Size;
		ULONG Flags;
		USHORT Index;
		USHORT NameLength;
		USHORT LoadCount;
		USHORT ModuleNameOffset;
		char ImageName[256];
	} SYSTEM_MODULE_INFORMATION_ENTRY64, *PSYSTEM_MODULE_INFORMATION_ENTRY64;



	typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY32
	{
		ULONG  Reserved[2];
		ULONG  Base;
		ULONG  Size;
		ULONG  Flags;
		USHORT Index;
		USHORT Unknown;
		USHORT LoadCount;
		USHORT ModuleNameOffset;
		CHAR   ImageName[256];
	} SYSTEM_MODULE_INFORMATION_ENTRY32, *PSYSTEM_MODULE_INFORMATION_ENTRY32;

#ifdef _WIN64
#define SYSTEM_MODULE_INFORMATION_ENTRY SYSTEM_MODULE_INFORMATION_ENTRY64
#else
#define SYSTEM_MODULE_INFORMATION_ENTRY SYSTEM_MODULE_INFORMATION_ENTRY32
#endif


	typedef struct _SYSTEM_MODULE_INFORMATION
	{
		ULONG Count;
		SYSTEM_MODULE_INFORMATION_ENTRY Module[1];
	} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;
#define SystemModuleInformation 11
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
	pfnZwQuerySystemInformation ZwQuerySystemInformationAddress = NULL;
	PSYSTEM_MODULE_INFORMATION  SystemModuleInformationPoint;
	ULONG NeedSize, BufferSize = 0x5000;
	PVOID Buffer = NULL;
	NTSTATUS bOk;
	ZwQuerySystemInformationAddress = (pfnZwQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"), "ZwQuerySystemInformation");
	do
	{
		Buffer = malloc(BufferSize);
		if (Buffer == NULL)
		{
			return 0;
		}
		bOk = ZwQuerySystemInformationAddress(SystemModuleInformation, Buffer, BufferSize, &NeedSize);
		if (bOk == STATUS_INFO_LENGTH_MISMATCH)
		{
			free(Buffer);
			BufferSize *= 2;
		}
		else if (!NT_SUCCESS(bOk))
		{
			free(Buffer);
			return 0;
		}
	} while (bOk == STATUS_INFO_LENGTH_MISMATCH);
	SystemModuleInformationPoint = (PSYSTEM_MODULE_INFORMATION)Buffer;
	ULONG_PTR Address = (ULONG_PTR)(SystemModuleInformationPoint->Module[0].Base);

	if (szNtosName != NULL)
	{
		//*////////////////////////////////////////////////////////////////////////
		memcpy(
			szNtosName,
			SystemModuleInformationPoint->Module[0].ImageName + SystemModuleInformationPoint->Module[0].ModuleNameOffset,
			strlen(SystemModuleInformationPoint->Module[0].ImageName + SystemModuleInformationPoint->Module[0].ModuleNameOffset)
		);
	}

	free(Buffer);

	return Address;
}

char *Strcat(char *Str1, char *Str2)
{
	DWORD dwLen = (DWORD)(strlen(Str1) + strlen(Str2) + 1);
	char* Str3 = (char*)malloc(dwLen);
	memcpy(Str3, Str1, strlen(Str1));
	memcpy(Str3 + strlen(Str1), Str2, strlen(Str2) + 1);
	return Str3;
}

CHAR* GetSystemDir()
{
	char* szPath;
	szPath = (char *)malloc(20);
	memset(szPath, 0, 20);
	GetWindowsDirectoryA(szPath, 20);
	return Strcat(szPath, "\\system32\\");
}

CHAR* LoadDllContext(char* szFileName)
{
	DWORD dwReadWrite, LenOfFile = FileLen(szFileName);
	HANDLE hFile = CreateFileA(szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		PCHAR Buffer = (PCHAR)malloc(LenOfFile);
		SetFilePointer(hFile, 0, 0, FILE_BEGIN);
		ReadFile(hFile, Buffer, LenOfFile, &dwReadWrite, 0);
		CloseHandle(hFile);
		return Buffer;
	}
	return NULL;
}

DWORD FileLen(char* szFileName)
{
	WIN32_FIND_DATAA FileInfo = { 0 };
	DWORD FileSize = 0;
	HANDLE hFind;
	hFind = FindFirstFileA(szFileName, &FileInfo);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FileSize = FileInfo.nFileSizeLow;
		FindClose(hFind);
	}
	return FileSize;
}

int Reloc(ULONG_PTR NewBase, ULONG_PTR OrigBase)
{
	PIMAGE_DOS_HEADER		DosHeader;
	PIMAGE_NT_HEADERS		NtHeader;
	PIMAGE_BASE_RELOCATION	RelocTable;
	ULONG i, dwOldProtect;
	DosHeader = (PIMAGE_DOS_HEADER)NewBase;
	if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return 0;
	}
	NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)NewBase + DosHeader->e_lfanew);
	if (NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)//�Ƿ�����ض�λ��
	{
		RelocTable = (PIMAGE_BASE_RELOCATION)((ULONG_PTR)NewBase + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		do
		{
			ULONG	NumOfReloc = (RelocTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
			SHORT	MiniOffset = 0;
			PUSHORT RelocData = (PUSHORT)((ULONG_PTR)RelocTable + sizeof(IMAGE_BASE_RELOCATION));
			for (i = 0; i<NumOfReloc; i++)
			{
				PULONG_PTR RelocAddress;//��Ҫ�ض�λ�ĵ�ַ

				if (((*RelocData) >> 12) == IMAGE_REL_BASED_DIR64 || ((*RelocData) >> 12) == IMAGE_REL_BASED_HIGHLOW)//�ж��ض�λ�����Ƿ�ΪIMAGE_REL_BASED_HIGHLOW[32]��IMAGE_REL_BASED_DIR64[64]
				{

					MiniOffset = (*RelocData) & 0xFFF;//Сƫ��

					RelocAddress = (PULONG_PTR)(NewBase + RelocTable->VirtualAddress + MiniOffset);

					VirtualProtect((PVOID)RelocAddress, sizeof(ULONG_PTR), PAGE_EXECUTE_READWRITE, &dwOldProtect);

					*RelocAddress = *RelocAddress + OrigBase - NtHeader->OptionalHeader.ImageBase;

					VirtualProtect((PVOID)RelocAddress, sizeof(ULONG_PTR), dwOldProtect, &dwOldProtect);
				}
				//��һ���ض�λ����
				RelocData++;
			}
			//��һ���ض�λ��
			RelocTable = (PIMAGE_BASE_RELOCATION)((ULONG_PTR)RelocTable + RelocTable->SizeOfBlock);
		} while (RelocTable->VirtualAddress);
		return TRUE;
	}
	return FALSE;
}

CHAR *GetTempNtdll()
{
	char *szPath;
	szPath = (char*)malloc(260);
	memset(szPath, 0, 260);
	GetTempPathA(260, szPath);   //û���ͷ��ڴ�  
	return Strcat(szPath, "ntdll.dll");
}

