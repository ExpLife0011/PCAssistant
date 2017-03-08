#include "stdafx.h"
#include "DetailFunc.h"
#include "Common.h"


WIN_VERSION WinVersion;

COLUMN_STRUCT g_Column_ProcessDetail[] =
{
	{ L"��Ŀ���",				150 },
	{ L"��Ŀ����",				440 }
};

WCHAR g_Item_ProcessDetail[][260] =
{
	L"ӳ������",
	L"����ID",
	L"������ID",
	L"ӳ���ļ�·��",
	L"EPROCESS",
	L"Ӧ�ò����",
	L"����ʱ��",
	L"����ʱ��",
	L"�޸�ʱ��",
	L"����ʱ��",
	L"�ļ���С",
	L"�ļ��汾",
	L"���̻�������",
	L"����������",
	L"���̵�ǰĿ¼",
	L"�ļ�����",
	L"�ļ�����"
};

enum PROCESS_DETAIL_ITEM
{
	PROCESS_DETAIL_ITEM_FILENAME = 0,
	PROCESS_DETAIL_ITEM_PID,
	PROCESS_DETAIL_ITEM_PPID,
	PROCESS_DETAIL_ITEM_PATH,
	PROCESS_DETAIL_ITEM_EPROCESS,
	PROCESS_DETAIL_ITEM_USERACCESS,
	PROCESS_DETAIL_ITEM_RUNTIME,
	PROCESS_DETAIL_ITEM_CREATETIME,
	PROCESS_DETAIL_ITEM_MOTIFITIME,
	PROCESS_DETAIL_ITEM_ACCESSTIME,
	PROCESS_DETAIL_ITEM_FILESIZE,
	PROCESS_DETAIL_ITEM_FILEVERSION,
	PROCESS_DETAIL_ITEM_PEB,
	PROCESS_DETAIL_ITEM_CMD,
	PROCESS_DETAIL_ITEM_CURDIR,
	PROCESS_DETAIL_ITEM_COMPANY,
	PROCESS_DETAIL_ITEM_DESCRIPTOR
};

typedef long(__fastcall *pfnRtlAdjustPrivilege64)(ULONG, ULONG, ULONG, PVOID);
pfnRtlAdjustPrivilege64 RtlAdjustPrivilege;

typedef
	NTSTATUS(__stdcall *pfnNtQueryInformationProcess)(HANDLE,
		UINT,
		PVOID,
		ULONG,
		PULONG
		);

pfnNtQueryInformationProcess NtQueryInformationProcessAddress = NULL;

UINT g_Column_ProcessDetail_Count = 2;	  //�����б�����
UINT g_Item_ProcessDetail_Count = 17;

VOID InitProcessDetailList(CListCtrl * m_ListCtrl)
{
	while (m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0; i<g_Column_ProcessDetail_Count; i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_ProcessDetail[i].szTitle, LVCFMT_LEFT, (int)(g_Column_ProcessDetail[i].nWidth));
	}
}


VOID LoadProcessDetailList(PPROCESSINFO ProcessInfo, CListCtrl *m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	UINT i;
	for (i = 0; i<g_Item_ProcessDetail_Count; i++)
	{
		m_ListCtrl->InsertItem(i, g_Item_ProcessDetail[i]);
	}

	WCHAR Temp[20] = { 0 };

	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_FILENAME, 1, ProcessInfo->Name);

	swprintf_s(Temp, L"%d", ProcessInfo->Pid);
	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_PID, 1, Temp);
	ZeroMemory(Temp, sizeof(WCHAR) * 20);

	swprintf_s(Temp, L"%d", ProcessInfo->PPid);
	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_PPID, 1, Temp);
	ZeroMemory(Temp, sizeof(WCHAR) * 20);

	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_PATH, 1, ProcessInfo->Path);

	swprintf_s(Temp, L"0x%p", ProcessInfo->Eprocess);
	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_EPROCESS, 1, Temp);
	ZeroMemory(Temp, sizeof(WCHAR) * 20);

	if (ProcessInfo->UserAccess == FALSE)
	{
		m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_USERACCESS, 1, L"�ܾ�");
	}
	else
	{
		m_ListCtrl->SetItemText(5, 1, L"����");
	}

	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_RUNTIME, 1, L"1970/01/01 00:00:00");

	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_COMPANY, 1, ProcessInfo->CompanyName);

	CString Detail = ProcessInfo->Path;
	Detail = GetFileDescription(Detail);
	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_DESCRIPTOR, 1, Detail.GetBuffer());

	CFileStatus Status;
	if (CFile::GetStatus(ProcessInfo->Path, Status))
	{
		CTime CreateTime = Status.m_ctime;
		CTime ModifyTime = Status.m_mtime;
		CTime AccessTime = Status.m_atime;

		CString strCreateTime;
		strCreateTime.Format(
			L"%04d/%02d/%02d %02d:%02d:%02d",
			CreateTime.GetYear(),
			CreateTime.GetMonth(),
			CreateTime.GetDay(),
			CreateTime.GetHour(),
			CreateTime.GetMinute(),
			CreateTime.GetSecond());

		CString strModifyTime;
		strModifyTime.Format(
			L"%04d/%02d/%02d %02d:%02d:%02d",
			ModifyTime.GetYear(),
			ModifyTime.GetMonth(),
			ModifyTime.GetDay(),
			ModifyTime.GetHour(),
			ModifyTime.GetMinute(),
			ModifyTime.GetSecond());

		CString strAccessTime;
		strAccessTime.Format(
			L"%04d/%02d/%02d %02d:%02d:%02d",
			AccessTime.GetYear(),
			AccessTime.GetMonth(),
			AccessTime.GetDay(),
			AccessTime.GetHour(),
			AccessTime.GetMinute(),
			AccessTime.GetSecond());

		m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_CREATETIME, 1, strCreateTime.GetBuffer());
		m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_MOTIFITIME, 1, strModifyTime.GetBuffer());
		m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_ACCESSTIME, 1, strAccessTime.GetBuffer());
	}
	else
	{

	}


	HANDLE hFile = CreateFile(
		ProcessInfo->Path,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CString strFileSize;
		LARGE_INTEGER FileSize;
		if (GetFileSizeEx(hFile, &FileSize))
		{
			strFileSize.Format(L"%d KB", FileSize.QuadPart / 1024);
		}
		CloseHandle(hFile);

		m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_FILESIZE, 1, strFileSize.GetBuffer());
	}
	else
	{
	}



	//////////////////////////////////////////////////////////////////////////
	//�汾

	TCHAR szVersionBuffer[8192] = _T("");
	DWORD dwVerSize = 0;
	DWORD dwHandle = 0;
	QWORD dwVersion = 0;

	dwVerSize = GetFileVersionInfoSize(ProcessInfo->Path, &dwHandle);

	if (dwVerSize)
	{
		if (GetFileVersionInfo(ProcessInfo->Path, dwHandle, dwVerSize, szVersionBuffer))
		{
			VS_FIXEDFILEINFO* Infor;
			unsigned int nInfoLen;
			if (VerQueryValue(szVersionBuffer, _T("\\"), (void**)&Infor, &nInfoLen))
			{
				dwVersion = Infor->dwFileVersionMS;
				dwVersion = dwVersion << 32;
				dwVersion |= Infor->dwFileVersionLS;
			}
		}
	}

	if (dwVersion)
	{
		CString strFileVersion;
		DWORD dwV1, dwV2, dwV3, dwV4;
		QWORD dwStackVersion = dwVersion;

		dwV1 = (DWORD)(dwStackVersion & 0xffff);
		dwStackVersion >>= 16;
		dwV2 = (DWORD)(dwStackVersion & 0xffff);
		dwStackVersion >>= 16;
		dwV3 = (DWORD)(dwStackVersion & 0xffff);
		dwStackVersion >>= 16;
		dwV4 = (DWORD)(dwStackVersion & 0xffff);

		strFileVersion.Format(L"%d.%d.%d.%d", dwV4, dwV3, dwV2, dwV1);

		m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_FILEVERSION, 1, strFileVersion.GetBuffer());

	}
	else
	{
	}


	//////////////////////////////////////////////////////////////////////////
	//PEB

	CString strPEB;

	strPEB = GetProcessPebAddress((DWORD)ProcessInfo->Pid);

	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_PEB, 1, strPEB.GetBuffer());


	//////////////////////////////////////////////////////////////////////////
	//������

	CString strCmdLine;

	strCmdLine = GetProcessCmdLine((DWORD)ProcessInfo->Pid);

	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_CMD, 1, strCmdLine.GetBuffer());


	//////////////////////////////////////////////////////////////////////////
	//��ǰĿ¼

	CString strCurDir;

	strCurDir = GetProcessCurrentDirectory((DWORD)ProcessInfo->Pid);

	m_ListCtrl->SetItemText(PROCESS_DETAIL_ITEM_CURDIR, 1, strCurDir.GetBuffer());
}



CString GetFileDescription(CString strPath)
{
	CString strDescription;

	if (!strPath.IsEmpty() && PathFileExists(strPath))
	{
		LPWSTR lpstrFilename = (LPWSTR)(LPCWSTR)strPath;
		DWORD  dwSize = GetFileVersionInfoSize(lpstrFilename, NULL);

		if (dwSize)
		{
			LPVOID Buffer = malloc(sizeof(char)*dwSize);
			if (Buffer)
			{
				if (GetFileVersionInfo(lpstrFilename, 0, dwSize, Buffer))
				{
					char* VerValue = NULL;
					UINT  nSize = 0;
					if (VerQueryValue(Buffer, L"\\VarFileInfo\\Translation", (LPVOID*)&VerValue, &nSize))
					{
						CString strSubBlock, strTranslation;
						strDescription.Format(L"000%x", *((unsigned short int *)VerValue));
						strTranslation = strDescription.Right(4);
						strDescription.Format(L"000%x", *((unsigned short int *)&VerValue[2]));
						strTranslation += strDescription.Right(4);

						strSubBlock.Format(L"\\StringFileInfo\\%s\\FileDescription", strTranslation);
						if (VerQueryValue(Buffer, strSubBlock.GetBufferSetLength(256), (LPVOID*)&VerValue, &nSize))
						{
							strSubBlock.ReleaseBuffer();
							strDescription.Format(L"%s", VerValue);
						}
					}
				}

				free(Buffer);
			}
		}
	}

	return strDescription;
}


CString GetProcessPebAddress(DWORD dwPid)
{
	CString szRet = L"";
	LONG                      Status;
	HANDLE                    hProcess = NULL;
	BOOL                      bRet = FALSE;
	LPTHREAD_START_ROUTINE    FuncAddress = NULL;
	DWORD                     dwRetVal = 0;


	WinVersion = GetWindowsVersion();

	switch (WinVersion)
	{
	case Windows7:   //ע������������Ե���64λ��Win7
	{
		PROCESS_BASIC_INFORMATION64 pbi;

#ifdef _UNICODE
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif	
		if (FuncAddress == NULL)
		{
			bRet = FALSE;

			break;
		}

		NtQueryInformationProcessAddress = (pfnNtQueryInformationProcess)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")), "NtQueryInformationProcess");

		if (NtQueryInformationProcessAddress == NULL)
		{
			bRet = FALSE;

			break;
		}


		RtlAdjustPrivilege = (pfnRtlAdjustPrivilege64)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")), "RtlAdjustPrivilege");

		if (RtlAdjustPrivilege == NULL)
		{
			bRet = FALSE;

			break;
		}
		RtlAdjustPrivilege(20, 1, 0, &dwRetVal);

		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);

		if (!hProcess)
		{
			bRet = FALSE;

			break;
		}

		Status = NtQueryInformationProcessAddress(hProcess,
			ProcessBasicInformation,
			(PVOID)&pbi,
			sizeof(PROCESS_BASIC_INFORMATION64),
			NULL
		);

		if (Status)
		{

			bRet = FALSE;

			break;
		}

		else
		{
			szRet.Format(L"0x%p", pbi.PebBaseAddress);

			bRet = TRUE;
		}


		break;
	}

	case WindowsXP:  //��������Ե�32λ��XP
	{


		PROCESS_BASIC_INFORMATION32 pbi;


#ifdef _UNICODE
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif	
		if (FuncAddress == NULL)
		{
			bRet = FALSE;

			break;
		}

		NtQueryInformationProcessAddress = (pfnNtQueryInformationProcess)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")), "NtQueryInformationProcess");

		if (NtQueryInformationProcessAddress == NULL)
		{
			bRet = FALSE;

			break;
		}
		DebugPrivilege(SE_DEBUG_NAME, TRUE);
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
		DebugPrivilege(SE_DEBUG_NAME, FALSE);
		if (!hProcess)
		{
			bRet = FALSE;

			break;;
		}


		Status = NtQueryInformationProcessAddress(hProcess,
			ProcessBasicInformation,
			(PVOID)&pbi,
			sizeof(PROCESS_BASIC_INFORMATION32),
			NULL
		);

		if (Status)
		{

			bRet = FALSE;

			break;
		}

		else
		{
			szRet.Format(L"0x%x", pbi.PebBaseAddress);

			bRet = TRUE;
		}


		break;
	}
	}


	if (hProcess)
	{
		CloseHandle(hProcess);
	}
	if (bRet == FALSE)
	{
		szRet.Empty();
	}

	return szRet;
}

CString GetProcessCmdLine(DWORD dwPid)
{
	CString m_CmdLine;
	LPTHREAD_START_ROUTINE FuncAddress = NULL;
	DWORD                  dwRetVal = 0;
	HANDLE                 hProcess = NULL;
	LONG                   Status;


	PVOID		CmdBuffer = NULL;
	SIZE_T      dwReturn = 0;

	DWORD                     dwSize = 0;
	LPVOID                    lpAddress = NULL;
	BOOL                      bRet = FALSE;

	switch (WinVersion)
	{
	case Windows7:   //ע������������Ե���64λ��Win7
	{

		_PEB64        Peb;
		PROCESS_BASIC_INFORMATION64 pbi;
		PROCESS_PARAMETERS64        ProcParam;
#ifdef _UNICODE
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif	
		if (FuncAddress == NULL)
		{
			bRet = FALSE;

			break;
		}

		NtQueryInformationProcessAddress = (pfnNtQueryInformationProcess)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")), "NtQueryInformationProcess");

		if (NtQueryInformationProcessAddress == NULL)
		{
			bRet = FALSE;

			break;
		}


		RtlAdjustPrivilege = (pfnRtlAdjustPrivilege64)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")), "RtlAdjustPrivilege");

		if (RtlAdjustPrivilege == NULL)
		{
			bRet = FALSE;

			break;
		}
		RtlAdjustPrivilege(20, 1, 0, &dwRetVal);

		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);

		if (!hProcess)
		{
			bRet = FALSE;

			break;
		}


		Status = NtQueryInformationProcessAddress(hProcess,
			ProcessBasicInformation,
			(PVOID)&pbi,
			sizeof(PROCESS_BASIC_INFORMATION64),
			NULL
		);

		if (Status)
		{
			bRet = FALSE;

			break;
		}

		if (!ReadProcessMemory(hProcess,
			(PVOID)pbi.PebBaseAddress,
			&Peb,
			sizeof(_PEB64),
			&dwReturn))
		{
			bRet = FALSE;

			break;
		}



		if (!ReadProcessMemory(hProcess,
			Peb.ProcessParameters,
			&ProcParam,
			sizeof(PROCESS_PARAMETERS64),
			&dwReturn
		)
			)
		{
			bRet = FALSE;

			break;
		}

		lpAddress = ProcParam.CommandLine.Buffer;
		dwSize = ProcParam.CommandLine.Length;

		CmdBuffer = malloc(dwSize + sizeof(WCHAR));
		if (!CmdBuffer)
		{
			bRet = FALSE;

			break;
		}

		memset(CmdBuffer, 0, dwSize + sizeof(WCHAR));
		if (!ReadProcessMemory(hProcess,
			lpAddress,
			CmdBuffer,
			dwSize,
			&dwReturn))
		{
			bRet = FALSE;

			break;
		}
		bRet = TRUE;
		m_CmdLine = (WCHAR*)CmdBuffer;


		break;
	}

	case WindowsXP:  //��������Ե�32λ��XP
	{

		_PEB32        Peb;
		PROCESS_BASIC_INFORMATION32 pbi;
		PROCESS_PARAMETERS32        ProcParam;

#ifdef _UNICODE
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif	
		if (FuncAddress == NULL)
		{
			bRet = FALSE;

			break;
		}

		NtQueryInformationProcessAddress = (pfnNtQueryInformationProcess)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")), "NtQueryInformationProcess");

		if (NtQueryInformationProcessAddress == NULL)
		{
			bRet = FALSE;

			break;
		}
		DebugPrivilege(SE_DEBUG_NAME, TRUE);
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
		DebugPrivilege(SE_DEBUG_NAME, FALSE);
		if (!hProcess)
		{
			bRet = FALSE;

			break;;
		}


		Status = NtQueryInformationProcessAddress(hProcess,
			ProcessBasicInformation,
			(PVOID)&pbi,
			sizeof(PROCESS_BASIC_INFORMATION32),
			NULL
		);

		if (Status)
		{
			bRet = FALSE;

			break;;
		}

		if (!ReadProcessMemory(hProcess,
			(PVOID)pbi.PebBaseAddress,
			&Peb,
			sizeof(_PEB32),
			&dwReturn))
		{
			bRet = FALSE;

			break;;
		}



		if (!ReadProcessMemory(hProcess,
			Peb.ProcessParameters,
			&ProcParam,
			sizeof(PROCESS_PARAMETERS32),
			&dwReturn
		)
			)
		{
			bRet = FALSE;

			break;
		}

		lpAddress = ProcParam.CommandLine.Buffer;
		dwSize = ProcParam.CommandLine.Length;

		CmdBuffer = malloc(dwSize + sizeof(WCHAR));
		if (!CmdBuffer)
		{
			bRet = FALSE;

			break;
		}

		memset(CmdBuffer, 0, dwSize + sizeof(WCHAR));
		if (!ReadProcessMemory(hProcess,
			lpAddress,
			CmdBuffer,
			dwSize,
			&dwReturn))
		{
			bRet = FALSE;

			break;
		}
		bRet = TRUE;
		m_CmdLine = (WCHAR*)CmdBuffer;

		break;
	}
	}




	if (hProcess != NULL)
	{
		CloseHandle(hProcess);
	}

	if (CmdBuffer)
	{
		free(CmdBuffer);
		CmdBuffer = NULL;
	}

	if (bRet == FALSE)
	{
		m_CmdLine.Empty();
	}


	return m_CmdLine;
}


CString GetProcessCurrentDirectory(DWORD dwPid)
{
	CString                   m_Dir;
	BOOL                      bRet = FALSE;
	LPTHREAD_START_ROUTINE    FuncAddress = NULL;
	DWORD                     dwRetVal = 0;
	SIZE_T                    dwReturn = 0;
	HANDLE                    hProcess = NULL;
	NTSTATUS                  Status;
	PVOID                     lpAddress = NULL;
	ULONG                     ulSize = 0;
	WCHAR*                    CmdBuffer = NULL;
	switch (WinVersion)
	{
	case Windows7:   //ע������������Ե���64λ��Win7
	{

		_PEB64        Peb;
		PROCESS_BASIC_INFORMATION64 pbi;
		PROCESS_PARAMETERS64        ProcParam;

#ifdef _UNICODE
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif	
		if (FuncAddress == NULL)
		{
			bRet = FALSE;

			break;
		}

		NtQueryInformationProcessAddress = (pfnNtQueryInformationProcess)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")), "NtQueryInformationProcess");

		if (NtQueryInformationProcessAddress == NULL)
		{
			bRet = FALSE;

			break;
		}


		RtlAdjustPrivilege = (pfnRtlAdjustPrivilege64)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")), "RtlAdjustPrivilege");

		if (RtlAdjustPrivilege == NULL)
		{
			bRet = FALSE;

			break;
		}
		RtlAdjustPrivilege(20, 1, 0, &dwRetVal);

		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);

		if (!hProcess)
		{
			bRet = FALSE;

			break;
		}

		Status = NtQueryInformationProcessAddress(hProcess,
			ProcessBasicInformation,
			(PVOID)&pbi,
			sizeof(PROCESS_BASIC_INFORMATION64),
			NULL
		);

		if (Status)
		{
			bRet = FALSE;

			break;
		}

		if (!ReadProcessMemory(hProcess,
			(PVOID)pbi.PebBaseAddress,
			&Peb,
			sizeof(_PEB64),
			&dwReturn))
		{
			bRet = FALSE;

			break;
		}



		if (!ReadProcessMemory(hProcess,
			Peb.ProcessParameters,
			&ProcParam,
			sizeof(PROCESS_PARAMETERS64),
			&dwReturn
		)
			)
		{
			bRet = FALSE;

			break;
		}


		lpAddress = ProcParam.CurrentDirectory.Buffer;
		ulSize = ProcParam.CurrentDirectory.Length;

		CmdBuffer = (WCHAR*)malloc(ulSize + sizeof(WCHAR));
		if (!CmdBuffer)
		{
			bRet = FALSE;

			break;
		}

		memset(CmdBuffer, 0, ulSize + sizeof(WCHAR));
		if (!ReadProcessMemory(hProcess,
			lpAddress,
			CmdBuffer,
			ulSize,
			&dwReturn))
		{
			bRet = FALSE;

			break;
		}


		m_Dir = (WCHAR*)CmdBuffer;
		bRet = TRUE;

		break;
	}

	case WindowsXP:  //��������Ե�32λ��XP
	{
		_PEB32                      Peb = { 0 };
		PROCESS_PARAMETERS32        ProcParam = { 0 };
		PROCESS_BASIC_INFORMATION32 pbi;


#ifdef _UNICODE
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
		FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif	
		if (FuncAddress == NULL)
		{
			bRet = FALSE;

			break;
		}

		NtQueryInformationProcessAddress = (pfnNtQueryInformationProcess)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")), "NtQueryInformationProcess");

		if (NtQueryInformationProcessAddress == NULL)
		{
			bRet = FALSE;

			break;
		}
		DebugPrivilege(SE_DEBUG_NAME, TRUE);
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
		DebugPrivilege(SE_DEBUG_NAME, FALSE);
		if (!hProcess)
		{
			bRet = FALSE;

			break;
		}

		Status = NtQueryInformationProcessAddress(hProcess,
			ProcessBasicInformation,
			(PVOID)&pbi,
			sizeof(PROCESS_BASIC_INFORMATION32),
			NULL
		);

		if (Status)
		{
			bRet = FALSE;

			break;;
		}

		if (!ReadProcessMemory(hProcess,
			(PVOID)pbi.PebBaseAddress,
			&Peb,
			sizeof(_PEB32),
			&dwReturn))
		{
			bRet = FALSE;

			break;;
		}



		if (!ReadProcessMemory(hProcess,
			Peb.ProcessParameters,
			&ProcParam,
			sizeof(PROCESS_PARAMETERS32),
			&dwReturn
		)
			)
		{
			bRet = FALSE;

			break;
		}

		lpAddress = ProcParam.CurrentDirectory.Buffer;
		ulSize = ProcParam.CurrentDirectory.Length;

		CmdBuffer = (WCHAR*)malloc(ulSize + sizeof(WCHAR));
		if (!CmdBuffer)
		{
			bRet = FALSE;

			break;
		}

		memset(CmdBuffer, 0, ulSize + sizeof(WCHAR));
		if (!ReadProcessMemory(hProcess,
			lpAddress,
			CmdBuffer,
			ulSize,
			&dwReturn))
		{
			bRet = FALSE;

			break;
		}


		m_Dir = (WCHAR*)CmdBuffer;
		bRet = TRUE;

		break;
	}
	}


	if (hProcess)
	{
		CloseHandle(hProcess);
	}

	if (CmdBuffer)
	{
		free(CmdBuffer);
		CmdBuffer = NULL;
	}

	if (bRet == FALSE)
	{
		m_Dir.Empty();
	}

	return m_Dir;
}