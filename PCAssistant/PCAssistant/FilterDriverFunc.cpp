#include "stdafx.h"
#include "FilterDriverFunc.h"
#include "ProcessFunc.h"
#include "Common.h"
#include <vector>

#include "ProcessFunc.h"

using namespace std;

extern HANDLE g_hDevice;
extern WIN_VERSION WinVersion;
extern BOOL bIsChecking;

vector<FILTER_INFO> m_FilterInfor;

COLUMN_STRUCT g_Column_FilterDriver[] =
{
	{ L"����",					80 },
	{ L"�����豸����",			125 },
	{ L"������������",			125 },
	{ L"������������",			125 },
	{ L"��������·��",			220 },
	{ L"��Ʒ����",				125 }
};

UINT g_Column_FilterDriver_Count = 6;

WCHAR szFilterType[][260] = {
	L"Unkonw",
	L"File",
	L"Disk",
	L"Volume",
	L"Keyboard",
	L"Mouse",
	L"I8042prt",
	L"Tcpip",
	L"NDIS",
	L"PnpManager"
	L"Tdx",
	L"RAW"
};

VOID InitFilterDriverList(CListCtrl *m_ListCtrl)
{
	while (m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0; i<g_Column_FilterDriver_Count; i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_FilterDriver[i].szTitle, LVCFMT_LEFT, (int)(g_Column_FilterDriver[i].nWidth));
	}
}


VOID LoadFilterDriverList(CListCtrl *m_ListCtrl)
{
	if (bIsChecking == TRUE)
	{
		return;
	}

	// 	while(bIsChecking == TRUE)
	// 	{
	// 		Sleep(10);
	// 	}

	bIsChecking = TRUE;

	SendStatusDetail(L"�����������ڼ���...");
	SendStatusTip(L"��������");

	QueryFilterDriverList(m_ListCtrl);

	bIsChecking = FALSE;
}

VOID QueryFilterDriverList(CListCtrl *m_ListCtrl)
{
	ULONG_PTR ulCnt = 100;
	PFILTER_DRIVER FilterDriver = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	m_ListCtrl->DeleteAllItems();
	m_FilterInfor.clear();

	do
	{
		ULONG_PTR ulSize = sizeof(FILTER_DRIVER) + ulCnt * sizeof(FILTER_INFO);

		if (FilterDriver)
		{
			free(FilterDriver);
			FilterDriver = NULL;
		}

		FilterDriver = (PFILTER_DRIVER)malloc(ulSize);

		if (FilterDriver)
		{
			memset(FilterDriver, 0, ulSize);
			FilterDriver->ulCnt = ulCnt;

			bRet = DeviceIoControl(g_hDevice, IOCTL(IOCTL_SYSK_FILTERDRIVER),
				NULL,
				0,
				FilterDriver,
				(DWORD)ulSize,
				&ulReturnSize,
				NULL);
		}

		ulCnt = FilterDriver->ulCnt + 10;

	} while (!bRet && FilterDriver->ulRetCnt > FilterDriver->ulCnt);

	if (bRet &&
		FilterDriver->ulCnt >= FilterDriver->ulRetCnt)
	{
		for (ULONG i = 0; i < FilterDriver->ulRetCnt; i++)
		{
			m_FilterInfor.push_back(FilterDriver->Filter[i]);
		}
	}

	if (FilterDriver)
	{
		free(FilterDriver);
		FilterDriver = NULL;
	}

	if (bRet == FALSE)
	{
		SendStatusDetail(L"������������ʧ�ܡ�");

		return;
	}

	InsertFilterDriverItem(m_ListCtrl);
}

void InsertFilterDriverItem(CListCtrl* m_ListCtrl)
{
	ULONG_PTR ulFilterDriver = 0;

	for (vector<FILTER_INFO>::iterator itor = m_FilterInfor.begin(); itor != m_FilterInfor.end(); itor++)
	{
		CString strFileterDeviceObject, strType;


		switch (itor->Type)
		{
		case Disk:
		{
			CString strTemp("Disk");
			strType = strTemp;
			break;
		}
		case Volume:
		{
			CString strTemp("Volume");
			strType = strTemp;
			break;
		}
		case File:
		{
			CString strTemp("File");
			strType = strTemp;
			break;
		}
		case Keyboard:
		{
			CString strTemp("Keyboard");
			strType = strTemp;
			break;
		}
		case Mouse:
		{
			CString strTemp("Mouse");
			strType = strTemp;
			break;
		}
		case I8042prt:
		{
			CString strTemp("I8042prt");
			strType = strTemp;
			break;
		}
		case Tcpip:
		{
			CString strTemp("TCP/IP");
			strType = strTemp;
			break;
		}
		case NDIS:
		{
			CString strTemp("NDIS");
			strType = strTemp;
			break;
		}
		case PnpManager:
		{
			CString strTemp("PnpManager");
			strType = strTemp;
			break;
		}
		case Tdx:
		{
			CString strTemp("Tdx");
			strType = strTemp;
			break;
		}
		case RAW:
		{
			CString strTemp("RAW");
			strType = strTemp;
			break;
		}
		default:
		{
			CString strTemp("Unknow");
			strType = strTemp;
			break;
		}
		}


		int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strType);
		strFileterDeviceObject.Format(L"0x%08p", itor->FileterDeviceObject);
		m_ListCtrl->SetItemText(n, 1, strFileterDeviceObject);
		CString strFilterDriverName(itor->wzFilterDriverName);
		m_ListCtrl->SetItemText(n, 2, strFilterDriverName);

		CString strAttachedDriverName(itor->wzAttachedDriverName);
		m_ListCtrl->SetItemText(n, 3, strAttachedDriverName);

		CString strPath(itor->wzPath);
		if (wcsstr(strPath, L"SystemRoot") != NULL)
		{
			WCHAR wzTempDir[260] = { 0 };
			GetEnvironmentVariableW(L"windir", wzTempDir, MAX_PATH);
			WCHAR* Temp = strPath.LockBuffer() + wcslen(L"\\SystemRoot");
			if (wzTempDir[lstrlen(wzTempDir) - 1] == '\\')
			{
				Temp = Temp + 1;
			}
			wcscat_s(wzTempDir, Temp);
			strPath.UnlockBuffer();
			strPath = wzTempDir;
		}
		else if (wcsstr(strPath, L"\\??\\") != NULL)
		{
			WCHAR* Temp = strPath.LockBuffer() + wcslen(L"\\??\\");
			strPath = Temp;
		}
		m_ListCtrl->SetItemText(n, 4, strPath);

		CString strFileCorp = GetFileCompanyName(strPath);

		m_ListCtrl->SetItemText(n, 5, strFileCorp);

		ulFilterDriver++;

		CString StatusBarContext;
		StatusBarContext.Format(L"�����������ڼ��ء� ��������%d", ulFilterDriver);
		SendStatusDetail(StatusBarContext);
	}

	CString StatusBarContext;
	StatusBarContext.Format(L"��������������ɡ� ��������%d", ulFilterDriver);
	SendStatusDetail(StatusBarContext);

}


VOID RemoveFilterDriverItem(CListCtrl* m_ListCtrl)
{
	UNLOAD_FILTER UnloadFilter;

	int Index = m_ListCtrl->GetSelectionMark();

	if (Index<0)
	{
		return;
	}

	CString Temp = m_ListCtrl->GetItemText(Index, 1);

	swscanf_s(Temp.GetBuffer() + 2, L"%p", &UnloadFilter.DeviceObject);

	Temp = m_ListCtrl->GetItemText(Index, 0);

	int nFilterType = 0;

	for (nFilterType = 0; nFilterType < 10; nFilterType++)
	{
		if (_wcsicmp(szFilterType[nFilterType], Temp) == 0)
		{
			break;
		}
	}

	UnloadFilter.Type = (FILTER_TYPE)nFilterType;


	BOOL dwRet = FALSE;

	DWORD dwReturnSize = 0;
	dwRet = DeviceIoControl(g_hDevice, IOCTL(IOCTL_SYSK_FILTERUNLOAD),
		&UnloadFilter,
		sizeof(UNLOAD_FILTER),
		NULL,
		0,
		&dwReturnSize,
		NULL);

	if (dwRet)
	{
		QueryFilterDriverList(m_ListCtrl);
	}

	bIsChecking = FALSE;
}