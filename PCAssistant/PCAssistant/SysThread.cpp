#include "stdafx.h"
#include "SysThread.h"
#include "Common.h"
#include "KModuleFunc.h"
#include "ThreadFunc.h"
#include "SSDTFunc.h"
#include <vector>

#include "ProcessFunc.h"

using namespace std;


extern HANDLE g_hDevice;
extern WIN_VERSION WinVersion;
extern BOOL bIsChecking;

COLUMN_STRUCT g_Column_SysThread[] =
{
	{ L"�߳�ID",				50 },
	{ L"�̶߳���",				125 },
	{ L"Peb",					35 },
	{ L"���ȼ�",				55 },
	{ L"�߳����",				125 },
	{ L"�л�����",				65 },
	{ L"�߳�״̬",				100 },
	{ L"ģ���ļ�",				190 },
	{ L"��Ʒ����",				125 }
};

extern vector<THREAD_INFO> m_vectorThread;
extern vector<DRIVER_INFO> m_DriverList;
ULONG_PTR m_ulSYSThreadCount = 0;

UINT g_Column_SysThread_Count = 9;

VOID InitSysThreadList(CListCtrl *m_ListCtrl)
{
	while (m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0; i<g_Column_SysThread_Count; i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_SysThread[i].szTitle, LVCFMT_LEFT, (int)(g_Column_SysThread[i].nWidth));
	}
}

VOID LoadSysThreadList(CListCtrl *m_ListCtrl)
{
	if (bIsChecking == TRUE)
	{
		return;
	}

	bIsChecking = TRUE;

	SendStatusDetail(L"�ں��߳����ڼ���...");
	SendStatusTip(L"�ں��߳�");

	QuerySysThreadList(m_ListCtrl);

	bIsChecking = FALSE;
}

VOID QuerySysThreadList(CListCtrl *m_ListCtrl)
{
	m_ulSYSThreadCount = 0;

	m_vectorThread.clear();
	m_DriverList.clear();

	BOOL bRet = FALSE;

	bRet = EnumDriver();

	if (bRet == FALSE)
	{
		SendStatusDetail(L"����ģ�����ʧ�ܡ�");
		return;
	}

	bRet = GetSYSThread(m_ListCtrl);

	if (bRet == FALSE)
	{
		SendStatusDetail(L"�ں��̼߳���ʧ�ܡ�");
		return;
	}

	for (vector <THREAD_INFO>::iterator Iter = m_vectorThread.begin();
		Iter != m_vectorThread.end();
		Iter++)
	{
		THREAD_INFO ThreadInfor = *Iter;

		if (ThreadInfor.State == Terminated)
		{
			return;
		}

		CString strTid, strEThread, strTeb, strPriority, strWin32StartAddress, strContextSwitches, strState, strModule;

		strTid.Format(L"%d", ThreadInfor.Tid);
		strEThread.Format(L"0x%08p", ThreadInfor.Thread);
		if (ThreadInfor.Teb == 0)
		{
			strTeb = L"-";
		}
		else
		{
			strTeb.Format(L"0x%08p", ThreadInfor.Teb);
		}

		strPriority.Format(L"%d", ThreadInfor.Priority);
		strWin32StartAddress.Format(L"0x%08p", ThreadInfor.Win32StartAddress);
		strContextSwitches.Format(L"%d", ThreadInfor.ContextSwitches);

		strModule = GetDriverPath(ThreadInfor.Win32StartAddress);

		switch (ThreadInfor.State)
		{
		case Initialized:
		{
			strState = L"Initialized";
			break;
		}
		case Ready:
		{
			strState = L"Ready";
			break;
		}
		case Running:
		{
			strState = L"Running";
			break;

		}
		case Standby:
		{
			strState = L"Standby";
			break;

		}
		case Terminated:
		{
			strState = L"Terminated";
			break;

		}
		case Waiting:
		{
			strState = L"Waiting";
			break;

		}
		case Transition:
		{
			strState = L"Transition";
			break;
		}

		case DeferredReady:
		{
			strState = L"Deferred Ready";
			break;

		}
		case GateWait:
		{
			strState = L"Gate Wait";
			break;

		}
		default:
		{
			strState = L"UnKnown";
			break;
		}
		}

		int n = m_ListCtrl->GetItemCount();
		int j = m_ListCtrl->InsertItem(n, strTid);
		m_ListCtrl->SetItemText(j, ThreadObject, strEThread);
		m_ListCtrl->SetItemText(j, ThreadTeb, strTeb);
		m_ListCtrl->SetItemText(j, ThreadPriority, strPriority);
		m_ListCtrl->SetItemText(j, ThreadStartAddress, strWin32StartAddress);
		m_ListCtrl->SetItemText(j, ThreadSwitchTimes, strContextSwitches);
		m_ListCtrl->SetItemText(j, ThreadStatus, strState);
		m_ListCtrl->SetItemText(j, ThreadStartModule, strModule);


		CString strComp;
		strComp = GetFileCompanyName(strModule);

		m_ListCtrl->SetItemText(j, ThreadComp, strComp);
		m_ListCtrl->SetItemData(j, j);

		m_ulSYSThreadCount++;

		CString StatusBarContext;
		StatusBarContext.Format(L"�ں��߳����ڼ��ء� �߳�����%d", m_ulSYSThreadCount);

		SendStatusDetail(StatusBarContext);
	}

	CString StatusBarContext;
	StatusBarContext.Format(L"�ں��̼߳�����ɡ� �߳�����%d", m_ulSYSThreadCount);

	SendStatusDetail(StatusBarContext);

}

BOOL GetSYSThread(CListCtrl *m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	m_vectorThread.clear();


	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	ULONG_PTR ulCnt = 1000;
	PALL_THREADS AllThreads = NULL;
	do
	{
		ULONG_PTR ulSize = 0;

		if (AllThreads)
		{
			free(AllThreads);
			AllThreads = NULL;
		}

		ulSize = sizeof(ALL_THREADS) + ulCnt * sizeof(THREAD_INFO);

		AllThreads = (PALL_THREADS)malloc(ulSize);
		if (!AllThreads)
		{
			break;
		}

		memset(AllThreads, 0, ulSize);

		bRet = DeviceIoControl(g_hDevice, IOCTL(IOCTL_SYSK_SYSTHREAD),
			NULL,
			0,
			AllThreads,
			(DWORD)ulSize,
			&ulReturnSize,
			NULL);

		ulCnt = AllThreads->nCnt + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && AllThreads->nCnt > 0)
	{
		for (ULONG i = 0; i < AllThreads->nCnt; i++)
		{
			m_vectorThread.push_back(AllThreads->Threads[i]);
		}
	}

	if (AllThreads)
	{
		free(AllThreads);
		AllThreads = NULL;
	}

	if (!m_vectorThread.empty())
	{
		return TRUE;
	}

	return FALSE;
}