#include "stdafx.h"
#include "ThreadFunc.h"
#include "Common.h"
#include <stdio.h>
#include <vector>

using namespace std;

vector<THREAD_INFO> m_vectorThread;
vector<MODULE_INFO> m_vectorModule;

extern ULONG_PTR g_ulProcessId;
extern HANDLE g_hDevice;
COLUMN_STRUCT g_Column_Thread[] =
{
	{ L"�߳�ID",			50 },
	{ L"ETHREAD",			125 },
	{ L"Teb",				125 },
	{ L"���ȼ�",			54 },
	{ L"�߳����",			125 },
	{ L"ģ��",				90 },
	{ L"�л�����",			68 },
	{ L"״̬",				50 }
};

UINT g_Column_Thread_Count = 8;	  //�����б�����

void InitThreadList(CListCtrl *m_ListCtrl)
{

	while (m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0; i<g_Column_Thread_Count; i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_Thread[i].szTitle, LVCFMT_LEFT, (int)(g_Column_Thread[i].nWidth));
	}
}

BOOL QueryProcessThread(CListCtrl *m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();
	m_vectorThread.clear();

	ULONG ulReturnSize = 0;
	BOOL bRet = FALSE;

	UINT m_Pid = (UINT)g_ulProcessId;



	if (m_Pid == 0)
	{
		return bRet;
	}

	m_vectorThread.clear();

	ULONG ulCount = 1000;
	PALL_THREADS Threads = NULL;

	do
	{
		ULONG ulSize = 0;

		if (Threads)
		{
			free(Threads);
			Threads = NULL;
		}

		ulSize = sizeof(ALL_THREADS) + ulCount * sizeof(THREAD_INFO);

		Threads = (PALL_THREADS)malloc(ulSize);
		if (!Threads)
		{
			break;
		}

		memset(Threads, 0, ulSize);


		bRet = DeviceIoControl(g_hDevice, IOCTL(IOCTL_PROC_PROCESSTHREAD),
			&m_Pid,
			sizeof(ULONG),
			Threads,
			ulSize,
			&ulReturnSize,
			NULL);

		ulCount = (ULONG)(Threads->nCnt + 100);

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && Threads->nCnt > 0)
	{
		for (ULONG i = 0; i<Threads->nCnt; i++)
		{
			m_vectorThread.push_back(Threads->Threads[i]);
		}
	}

	if (Threads)
	{
		free(Threads);
		Threads = NULL;
	}

	if (m_vectorThread.empty())
	{
		return FALSE;
	}

	//////////////////////////////////////////////////////////////////////////

	ulCount = 0x10;
	PALL_MODULES AllModules = NULL;

	do
	{

		ULONG ulSize = 0;

		if (AllModules)
		{
			free(AllModules);
			AllModules = NULL;
		}

		ulSize = sizeof(ALL_MODULES) + ulCount * sizeof(MODULE_INFO);

		AllModules = (PALL_MODULES)malloc(ulSize);
		if (!AllModules)
		{
			break;
		}

		memset(AllModules, 0, ulSize);


		bRet = DeviceIoControl(g_hDevice, IOCTL(IOCTL_PROC_PROCESSTHREADMODULE),
			&m_Pid,
			sizeof(ULONG_PTR),
			AllModules,
			ulSize,
			&ulReturnSize,
			NULL);


		ulCount = (ULONG)(AllModules->ulCount + 1000);

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);


	if (bRet && AllModules->ulCount > 0)
	{

		for (ULONG i = 0; i<AllModules->ulCount; i++)
		{
			MODULE_INFO Temp;

			Temp.Base = AllModules->Modules[i].Base;
			Temp.Size = AllModules->Modules[i].Size;
			CString szPath = TrimPath(AllModules->Modules[i].Path);

			wcsncpy_s(Temp.Path, MAX_PATH, szPath.GetBuffer(), szPath.GetLength());
			szPath.ReleaseBuffer();

			m_vectorModule.push_back(Temp);

		}
	}


	if (AllModules != NULL)
	{
		free(AllModules);
		AllModules = NULL;
	}

	//////////////////////////////////////////////////////////////////////////

	for (vector <THREAD_INFO>::iterator Iter = m_vectorThread.begin();
		Iter != m_vectorThread.end();
		Iter++)
	{
		THREAD_INFO ThreadInfor = (THREAD_INFO)*Iter;
		if (ThreadInfor.State == Terminated)
		{
			return FALSE;
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

		strModule = GetModulePathByThreadStartAddress(ThreadInfor.Win32StartAddress);

		if (strModule.GetLength() <= 1)
		{
			strModule = L"\\ ";
		}

		WCHAR* Temp = NULL;

		Temp = wcsrchr(strModule.GetBuffer(), L'\\');

		if (Temp != NULL)
		{
			Temp++;
		}

		strModule = Temp;

		switch (ThreadInfor.State)
		{
		case Initialized:
		{
			strState = L"Ԥ��";
			break;
		}
		case Ready:
		{
			strState = L"����";
			break;
		}
		case Running:
		{
			strState = L"����";
			break;

		}
		case Standby:
		{
			strState = L"����";
			break;

		}
		case Terminated:
		{
			strState = L"��ֹ";
			break;

		}
		case Waiting:
		{
			strState = L"�ȴ�";
			break;

		}
		case Transition:
		{
			strState = L"����";
			break;
		}

		case DeferredReady:
		{
			strState = L"�ӳپ���";
			break;

		}
		case GateWait:
		{
			strState = L"�ŵȴ�";
			break;

		}
		default:
		{
			strState = L"δ֪";
			break;
		}
		}

		int n = m_ListCtrl->GetItemCount();
		int j = m_ListCtrl->InsertItem(n, strTid);
		m_ListCtrl->SetItemText(j, THREAD_COLUMN_ETHREAD, strEThread);
		m_ListCtrl->SetItemText(j, THREAD_COLUMN_TEB, strTeb);
		m_ListCtrl->SetItemText(j, THREAD_COLUMN_PRIORITY, strPriority);
		m_ListCtrl->SetItemText(j, THREAD_COLUMN_ENTRANCE, strWin32StartAddress);
		m_ListCtrl->SetItemText(j, THREAD_COLUMN_COUNTER, strContextSwitches);
		m_ListCtrl->SetItemText(j, THREAD_COLUMN_STATUS, strState);
		m_ListCtrl->SetItemText(j, THREAD_COLUMN_MODULE, strModule);
		m_ListCtrl->SetItemData(j, j);

	}

	return bRet;
}


CString GetModulePathByThreadStartAddress(ULONG_PTR ulBase)
{
	CString szRet = L"";


	for (vector <MODULE_INFO>::iterator Iter = m_vectorModule.begin();
		Iter != m_vectorModule.end();
		Iter++)
	{
		MODULE_INFO entry = *Iter;
		if (ulBase >= entry.Base && ulBase <= (entry.Base + entry.Size))
		{
			szRet = entry.Path;
		}
	}
	//���������ѭ�� ��˵�����ں�ģ��
	return szRet;
}
