#include "stdafx.h"
#include "WindowFunc.h"
#include "Common.h"
#include "resource.h"

#include <vector>

using namespace std;


vector<WND_INFO> m_vectorWnds;

extern ULONG_PTR g_ulProcessId;
extern HANDLE g_hDevice;

COLUMN_STRUCT g_Column_Window[] =
{
	{ L"���ھ��",			80 },
	{ L"���ڱ���",			140 },
	{ L"��������",			140 },
	{ L"���ڿɼ���",		90 },
	{ L"�߳�ID",			70 },
	{ L"����ID",			70 }
};

UINT g_Column_Window_Count = 6;	  //�����б�����

void InitWindowList(CListCtrl *m_ListCtrl)
{

	while (m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0; i<g_Column_Window_Count; i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_Window[i].szTitle, LVCFMT_LEFT, (int)(g_Column_Window[i].nWidth));
	}
}

VOID QueryProcessWindow(CListCtrl *m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	BOOL bRet = FALSE;
	ULONG ulReturnSize = 0;
	m_vectorWnds.clear();

	PALL_WNDS WndInfo = NULL;
	ULONG ulCount = 1000;

	if (g_ulProcessId <= 4)
	{
		return;
	}

	do
	{
		ULONG ulSize = sizeof(ALL_WNDS) + ulCount * sizeof(WND_INFO);

		if (WndInfo)
		{
			free(WndInfo);
			WndInfo = NULL;
		}

		WndInfo = (PALL_WNDS)malloc(ulSize);
		if (!WndInfo)
		{
			break;
		}

		memset(WndInfo, 0, ulSize);


		bRet = DeviceIoControl(g_hDevice,
			IOCTL(IOCTL_PROC_PROCESSWINDOW),
			&g_ulProcessId,
			sizeof(ULONG),
			WndInfo,
			ulSize,
			&ulReturnSize,
			NULL);

		ulCount = WndInfo->nCnt + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && WndInfo->nCnt > 0)
	{
		for (ULONG i = 0; i<WndInfo->nCnt; i++)
		{
			m_vectorWnds.push_back(WndInfo->WndInfo[i]);
		}
	}

	if (WndInfo)
	{
		free(WndInfo);
		WndInfo = NULL;
	}



	if (m_vectorWnds.empty())
	{
		return;
	}


	//////////////////////////////////////////////////////////////////////////

	for (vector <WND_INFO>::iterator Iter = m_vectorWnds.begin();
		Iter != m_vectorWnds.end();
		Iter++)
	{
		AddWndItem(*Iter, FALSE, m_ListCtrl);
	}
}

void AddWndItem(WND_INFO WndInfor, BOOL bAll, CListCtrl *m_ListCtrl)
{


	WCHAR szClassName[MAX_PATH] = { 0 };
	WCHAR szWindowsText[MAX_PATH] = { 0 };

	CString  strhWnd;
	CString  strTid;
	CString  strPid;
	CString  strVisable;

	::GetClassName(WndInfor.hWnd, szClassName, MAX_PATH);
	::GetWindowText(WndInfor.hWnd, szWindowsText, MAX_PATH);


	if (::IsWindowVisible(WndInfor.hWnd))
	{
		strVisable = L"�ɼ�";
	}
	else
	{
		strVisable = L"-";
	}


	if (bAll == TRUE)
	{

	}
	else
	{
		if (WndInfor.uPid != g_ulProcessId)
		{
			return;
		}
	}

	strhWnd.Format(L"0x%08X", WndInfor.hWnd);
	strTid.Format(L"%d", WndInfor.uTid);
	strPid.Format(L"%d", WndInfor.uPid);
	int n = m_ListCtrl->GetItemCount();
	int j = m_ListCtrl->InsertItem(n, strhWnd);
	m_ListCtrl->SetItemText(j, 1, szWindowsText);
	m_ListCtrl->SetItemText(j, 2, szClassName);
	m_ListCtrl->SetItemText(j, 3, strVisable);
	m_ListCtrl->SetItemText(j, 4, strTid);
	m_ListCtrl->SetItemText(j, 5, strPid);
	m_ListCtrl->SetItemData(j, j);
}
