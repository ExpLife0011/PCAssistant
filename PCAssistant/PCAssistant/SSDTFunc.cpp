#include "stdafx.h"
#include "SSDTFunc.h"
#include "KModuleFunc.h"
#include "Common.h"
#include <vector>

using namespace std;

extern vector<DRIVER_INFO> m_DriverList;

extern HANDLE g_hDevice;
extern BOOL bIsChecking;
extern WIN_VERSION WinVersion;

COLUMN_STRUCT g_Column_SSDT[] =
{
	{ L"���",					50 },
	{ L"��������",				145 },
	{ L"������ǰ��ַ",			125 },
	{ L"����ԭ��ַ",			125 },
	{ L"ģ���ļ�",				195 },
	{ L"״̬",					85 }
};

UINT g_Column_SSDT_Count = 6;	  //�����б�����

ULONG_PTR m_KiServiceTable = 0;
ULONG_PTR m_SSDTFuncCount = 0;
ULONG_PTR m_HookFuncCount = 0;
CHAR  m_szNtosName[512];
CHAR  m_szTempNtosName[512];
ULONG_PTR m_NtosImageBase;
ULONG_PTR m_NtosBase;
ULONG_PTR m_TempNtosBase;
BOOL  m_bSSDTOk = FALSE;

VOID InitSSDTList(CListCtrl *m_ListCtrl)
{
	while (m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0; i<g_Column_SSDT_Count; i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_SSDT[i].szTitle, LVCFMT_LEFT, (int)(g_Column_SSDT[i].nWidth));
	}

}


VOID LoadSSDTList(CListCtrl *m_ListCtrl)
{
	SendStatusDetail(L"SSDT���ڼ���...");
	SendStatusTip(L"SSDT");

	QuerySSDTList(m_ListCtrl);

	bIsChecking = FALSE;
}

VOID QuerySSDTList(CListCtrl *m_ListCtrl)
{


	SSDT_INFO si[0x1000] = { 0 };

	m_KiServiceTable = 0;
	m_HookFuncCount = 0;
	m_SSDTFuncCount = 0;

	m_DriverList.clear();

	BOOL bRet = FALSE;

	InitDataOfSSDT();

	bRet = EnumSSDT(si);

	if (bRet == FALSE)
	{
		SendStatusDetail(L"SSDT����ʧ�ܡ�");
		return;
	}

	bRet = EnumDriver();

	if (bRet == FALSE)
	{
		SendStatusDetail(L"SSDT����ʧ�ܡ�");
		return;
	}

	CString strIndex;

	int i = 0;
	int j = 0;
	for (i = 0; i<m_SSDTFuncCount; i++)
	{

		strIndex.Format(L"%d", si[i].Id);
		int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strIndex);

		CString strFuncName(si[i].szFuncName);
		m_ListCtrl->SetItemText(n, 1, strFuncName);

		CString strCurAddr;
		strCurAddr.Format(L"0x%p", si[i].CurAddr);
		m_ListCtrl->SetItemText(n, 2, strCurAddr);

		CString strOrigAddr;
		strOrigAddr.Format(L"0x%p", si[i].OriAddr);
		m_ListCtrl->SetItemText(n, 3, strOrigAddr);


		CString strType;
		if (si[i].OriAddr != si[i].CurAddr)
		{
			m_ListCtrl->SetItemData(n, 1);

			strType = L"SSDTHook";
		}
		else
		{
			strType = L"����";
		}


		m_ListCtrl->SetItemText(n, 5, strType);

		CString strPath;

		strPath = GetDriverPath(si[i].CurAddr);
		m_ListCtrl->SetItemText(n, 4, strPath);

		if (_wcsnicmp(L"����", strType, wcslen(L"����")) != 0)
		{
			j += 1;
		}

		CString StatusBarContext;
		StatusBarContext.Format(L"SSDT���ڼ��ء� SSDT������%d���ҹ�������%d", i + 1, j);

		SendStatusDetail(StatusBarContext);
	}

	CString StatusBarContext;
	StatusBarContext.Format(L"SSDT������ɡ� SSDT������%d���ҹ�������%d", m_SSDTFuncCount, m_HookFuncCount);

	SendStatusDetail(StatusBarContext);
}

BOOL InitDataOfSSDT()
{
	BOOL b1 = FALSE, b2 = FALSE;
	BOOL bOk = FALSE;
	b1 = GetNtosNameAndBase();
	b2 = GetNtosImageBase();

	m_TempNtosBase = (ULONG_PTR)LoadLibraryExA(m_szTempNtosName, 0, DONT_RESOLVE_DLL_REFERENCES);

	if (m_bSSDTOk == FALSE)
	{
		Reloc((ULONG_PTR)m_TempNtosBase, m_NtosBase);
		m_bSSDTOk = TRUE;
	}

	if (GetKiServiceTable() == FALSE)
	{
		return FALSE;
	}

	if (b1 && b2)
	{
		bOk = TRUE;
	}
	else
	{
		bOk = FALSE;
	}

	return bOk;
}

BOOL GetNtosNameAndBase()    //���Ntos��·���ͼ��ص�ַ
{
	char szFileName[260] = { 0 }, *szFullName;
	m_NtosBase = GetKernelBase(szFileName);
	szFullName = Strcat(GetSystemDir(), szFileName);
	strcpy_s(m_szNtosName, szFullName);
	return GetTempNtosName();
}

BOOL GetNtosImageBase()
{
	PIMAGE_NT_HEADERS NtHeader;
	PIMAGE_DOS_HEADER DosHeader;
	char* szNtosFileData = NULL;
	szNtosFileData = LoadDllContext(m_szTempNtosName);
	if (szNtosFileData == NULL)
	{
		return FALSE;
	}
	DosHeader = (PIMAGE_DOS_HEADER)szNtosFileData;
	NtHeader = (PIMAGE_NT_HEADERS)(szNtosFileData + DosHeader->e_lfanew);
	m_NtosImageBase = NtHeader->OptionalHeader.ImageBase;
	return TRUE;
}

BOOL GetTempNtosName()   //����Ntos��·�� ���п�������ntosxxxx.exe
{
	char *szPath;
	szPath = (char *)malloc(260);
	memset(szPath, 0, 260);
	GetTempPathA(260, szPath);
	szPath = Strcat(szPath, "ntosxxxx.exe");
	strcpy_s(m_szTempNtosName, szPath);

	if (CopyFileA(m_szNtosName, m_szTempNtosName, 0))
	{
		free(szPath);
		return TRUE;
	}
	else
	{
		free(szPath);
		return FALSE;
	}
}


BOOL GetKiServiceTable()
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	if (g_hDevice == NULL)
	{
		return FALSE;
	}

	dwRet = DeviceIoControl(
		g_hDevice,
		IOCTL(IOCTL_KRNL_KISRVTAB),
		NULL,
		0,
		&m_KiServiceTable,
		sizeof(ULONG_PTR),
		&dwReturnSize,
		NULL);


	if (dwRet == 0)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL EnumSSDT(SSDT_INFO* si)
{
	DWORD i = 0, SSDTFuncCount = 0;


	CHAR* szTempNtdll = NULL;
	DWORD dwLen = 0;
	DWORD dwStart = 0;
	DWORD dwEnd = 0;
	ULONG_PTR SSDTOriAddr = 0;
	ULONG_PTR SSDTCurAddr = 0;
	szTempNtdll = GetTempNtdll();

	if (!CopyFileA(Strcat(GetSystemDir(), "ntdll.dll"), szTempNtdll, 0))
	{
		return FALSE;
	}

	dwLen = FileLen(szTempNtdll);

	char szFuncStart[] = "ZwAcceptConnectPort", szFuncEnd[] = "ZwYieldExecution"; //ÿ��������֮�����\0

	char* szNtdll = LoadDllContext(szTempNtdll);
	char* szTemp = szNtdll;
	for (i = 0; i<dwLen; i++)
	{
		if (memcmp(szTemp + i, szFuncStart, strlen(szFuncStart)) == 0)
		{
			dwStart = i;
		}
		if (memcmp(szTemp + i, szFuncEnd, strlen(szFuncEnd)) == 0)
		{
			dwEnd = i;
			break;
		}
	}
	szTemp = szTemp + dwStart;
	//���ﲻ����ʾ�����һ������
	while (strcmp(szTemp, szFuncEnd) != 0)
	{
		DWORD dwFuncIndex = GetSSDTFunctionIndex(szTemp);
		if (dwFuncIndex<0x1000)
		{
			SSDTOriAddr = GetFunctionOriginalAddress(dwFuncIndex);

			SendIoCodeSSDT(dwFuncIndex, &SSDTCurAddr);


			if (SSDTCurAddr != SSDTOriAddr)
			{
				m_HookFuncCount++;
			}

			szTemp[0] = 'N';
			szTemp[1] = 't';		//Ѱ�ҵ���Zw***������Ӧ����ʾNt***
		}
		else
		{

			dwFuncIndex = GetSpecialIndex(szTemp);
			SSDTOriAddr = GetFunctionOriginalAddress(dwFuncIndex);
			SendIoCodeSSDT(dwFuncIndex, &SSDTCurAddr);

			if (SSDTCurAddr != SSDTOriAddr)
			{
				m_HookFuncCount++;
			}
			szTemp[0] = 'N';
			szTemp[1] = 't';
		}
		si[SSDTFuncCount].Id = dwFuncIndex;
		si[SSDTFuncCount].CurAddr = SSDTCurAddr;
		si[SSDTFuncCount].OriAddr = SSDTOriAddr;
		strcpy_s(si[SSDTFuncCount].szFuncName, szTemp);
		szTemp = szTemp + strlen(szTemp) + 1;
		SSDTFuncCount++;
		dwFuncIndex = 0;
	}
	//��ʾ�����һ������
	DWORD dwFuncIndex = GetSSDTFunctionIndex(szTemp);
	SSDTOriAddr = GetFunctionOriginalAddress(dwFuncIndex);
	SendIoCodeSSDT(dwFuncIndex, &SSDTCurAddr);

	if (SSDTCurAddr != SSDTOriAddr)
	{
		m_HookFuncCount++;
	}
	szTemp[0] = 'N';
	szTemp[1] = 't';
	si[SSDTFuncCount].Id = dwFuncIndex;
	si[SSDTFuncCount].CurAddr = SSDTCurAddr;
	si[SSDTFuncCount].OriAddr = SSDTOriAddr;
	strcpy_s(si[SSDTFuncCount].szFuncName, szTemp);
	SSDTFuncCount++;

	m_SSDTFuncCount = SSDTFuncCount;
	DeleteFileA(szTempNtdll);
	DeleteFileA(m_szTempNtosName);
	free(szNtdll);

	return TRUE;
}


DWORD GetSSDTFunctionIndex(char *FunctionName)
{

	ULONG_PTR IndexOffset = 0;

	switch (WinVersion)
	{
	case Windows7:
	{
		IndexOffset = 4;

		break;
	}

	case WindowsXP:
	{

		IndexOffset = 1;

		break;
	}
	}
	return *(DWORD*)((PUCHAR)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), FunctionName) + IndexOffset);
}

ULONG_PTR GetFunctionOriginalAddress(ULONG_PTR dwIndex)	//ͨ��INDEX��ú���ԭʼ��ַ
{
	ULONG_PTR RVA = m_KiServiceTable - m_NtosBase;
	ULONG_PTR Temp = *(PULONG_PTR)(m_TempNtosBase + RVA + sizeof(ULONG_PTR)*(ULONG_PTR)dwIndex);
	return Temp;
}



BOOL SendIoCodeSSDT(DWORD dwFuncIndex, PULONG_PTR SSDTCurAddr)
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	//����IO ������
	dwRet = DeviceIoControl(g_hDevice, IOCTL(IOCTL_KRNL_SSDTLIST),
		&dwFuncIndex,
		sizeof(DWORD),
		SSDTCurAddr,
		sizeof(ULONG_PTR),
		&dwReturnSize,
		NULL);


	if (dwRet == 0)
	{
		return FALSE;
	}

	return TRUE;
}


DWORD GetSpecialIndex(char *FunctionName)
{
	switch (WinVersion)
	{
	case Windows7:
	{
		if (!_stricmp(FunctionName, "ZwQuerySystemTime"))
		{
			return 0x57;
		}
		return 0;
	}

	default:
		return 0;
	}
}

BOOL EnumDriver()
{
	ULONG ulReturnSize = 0;
	BOOL bRet = FALSE;

	m_DriverList.clear();

	ULONG ulCount = 1000;
	PALL_DRIVERS Drivers = NULL;

	if (g_hDevice == NULL)
	{
		MessageBox(NULL, L"�豸��ȡʧ��", L"Error", 0);
		return FALSE;
	}

	do
	{
		ULONG ulSize = 0;

		if (Drivers)
		{
			free(Drivers);
			Drivers = NULL;
		}

		ulSize = sizeof(ALL_DRIVERS) + ulCount * sizeof(DRIVER_INFO);

		Drivers = (PALL_DRIVERS)malloc(ulSize);
		if (!Drivers)
		{
			break;
		}

		memset(Drivers, 0, ulSize);


		bRet = DeviceIoControl(
			g_hDevice,
			IOCTL(IOCTL_MODU_MODULELIST),
			NULL,
			0,
			Drivers,
			ulSize,
			&ulReturnSize,
			NULL);

		ulCount = (ULONG)Drivers->ulCount + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && Drivers->ulCount > 0)
	{
		for (ULONG i = 0; i<Drivers->ulCount; i++)
		{
			FixDriverPath(&Drivers->Drivers[i]);
			m_DriverList.push_back(Drivers->Drivers[i]);
		}
	}

	if (Drivers)
	{
		free(Drivers);
		Drivers = NULL;
	}

	return bRet;
}


CString GetDriverPath(ULONG_PTR Address)
{
	CString strPath;

	for (vector<DRIVER_INFO>::iterator itor = m_DriverList.begin();
		itor != m_DriverList.end();
		itor++)
	{
		ULONG_PTR ulBase = itor->Base;
		ULONG_PTR ulEnd = itor->Base + itor->Size;

		if (Address >= ulBase && Address <= ulEnd)
		{
			strPath = itor->wzDriverPath;
			break;
		}
	}

	return strPath;
}
