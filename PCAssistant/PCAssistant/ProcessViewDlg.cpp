// ProcessViewDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCAssistant.h"
#include "ProcessViewDlg.h"
#include "afxdialogex.h"
#include "DetailFunc.h"
#include "ThreadFunc.h"
#include "PrivilegeFunc.h"
#include "HandleFunc.h"
#include "WindowFunc.h"
#include "ModuleFunc.h"
#include "MemoryFunc.h"


UINT nowType = 999;
ULONG_PTR g_ulProcessId = 0;
// CProcessViewDlg �Ի���

IMPLEMENT_DYNAMIC(CProcessViewDlg, CDialogEx)

CProcessViewDlg::CProcessViewDlg(ULONG_PTR ulViewType, PPROCESSINFO piProcInfo, CWnd* pParent)
	: CDialogEx(CProcessViewDlg::IDD, pParent)
{
	m_wParent = pParent;
	m_piProcInfo = piProcInfo;
	m_ulViewType = ulViewType;

	g_ulProcessId = m_piProcInfo->Pid;
}

CProcessViewDlg::~CProcessViewDlg()
{

}

void CProcessViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESSVIEWTYPE, m_ListType);
	DDX_Control(pDX, IDC_LIST_PROCESS_VIEW, m_ListCtrl);
}


BEGIN_MESSAGE_MAP(CProcessViewDlg, CDialogEx)
	ON_LBN_SELCHANGE(IDC_LIST_PROCESSVIEWTYPE, &CProcessViewDlg::OnLbnSelchangeListProcessviewtype)
	ON_COMMAND(ID_MENU_PROCESS_THREAD_REFRESH, &CProcessViewDlg::OnMenuProcessThreadRefresh)
	ON_COMMAND(ID_MENU_PROCESS_THREAD_CLOSE, &CProcessViewDlg::OnMenuProcessThreadClose)
END_MESSAGE_MAP()


// CProcessViewDlg ��Ϣ�������

BOOL CProcessViewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_ListType.InsertString(PROCVIEW_TYPE_DETAIL, L"����ӳ��");
	m_ListType.InsertString(PROCVIEW_TYPE_THREAD, L"�����߳�");
	m_ListType.InsertString(PROCVIEW_TYPE_PRIVILEGE, L"����Ȩ��");
	m_ListType.InsertString(PROCVIEW_TYPE_HANDLE, L"���̾��");
	m_ListType.InsertString(PROCVIEW_TYPE_WINDOW, L"���̴���");
	m_ListType.InsertString(PROCVIEW_TYPE_MODULE, L"����ģ��");
	m_ListType.InsertString(PROCVIEW_TYPE_MEMORY, L"�����ڴ�");

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CProcessViewDlg::OnLbnSelchangeListProcessviewtype()
{
	int nItem = m_ListType.GetCurSel();

	m_ulViewType = nItem;

	switch (m_ulViewType)
	{
	case PROCVIEW_TYPE_DETAIL:
	{
		if (nowType == PROCVIEW_TYPE_DETAIL)
		{
			return;
		}
		nowType = PROCVIEW_TYPE_DETAIL;

		CString Temp;
		Temp.Format(L"����ӳ�� - %s", m_piProcInfo->Name);

		SetWindowText(Temp.GetBuffer());

		InitProcessDetailList(&m_ListCtrl);

		LoadProcessDetailList(m_piProcInfo, &m_ListCtrl);

		break;
	}

	
	case PROCVIEW_TYPE_THREAD:
	{
		if (nowType == PROCVIEW_TYPE_THREAD)
		{
			return;
		}
		nowType = PROCVIEW_TYPE_THREAD;

		CString Temp;

		Temp.Format(L"�����߳� - %s", m_piProcInfo->Name);

		SetWindowText(Temp.GetBuffer());

		InitThreadList(&m_ListCtrl);

		QueryProcessThread(&m_ListCtrl);

		break;
	}

	
	case PROCVIEW_TYPE_PRIVILEGE:
	{
		if (nowType == PROCVIEW_TYPE_PRIVILEGE)
		{
			return;
		}
		nowType = PROCVIEW_TYPE_PRIVILEGE;

		CString Temp;
		Temp.Format(L"����Ȩ�� - %s", m_piProcInfo->Name);

		SetWindowText(Temp.GetBuffer());

		InitPrivilegeList(&m_ListCtrl);

		//CloseHandle(CreateThread(NULL, 0,
		//	(LPTHREAD_START_ROUTINE)HsQueryProcessPrivilege,&m_viewList, 0, NULL));

		QueryProcessPrivilege(&m_ListCtrl);

		break;
	}
	
	case PROCVIEW_TYPE_HANDLE:
	{
		if (nowType == PROCVIEW_TYPE_HANDLE)
		{
			return;
		}
		nowType = PROCVIEW_TYPE_HANDLE;

		CString Temp;
		Temp.Format(L"���̾�� - %s", m_piProcInfo->Name);

		SetWindowText(Temp.GetBuffer());

		InitProcessHandleList(&m_ListCtrl);

		//CloseHandle(CreateThread(NULL, 0,
		//	(LPTHREAD_START_ROUTINE)HsQueryProcessHandle,&m_viewList, 0, NULL));

		QueryProcessHandle(&m_ListCtrl);

		break;
	}

	
	case PROCVIEW_TYPE_WINDOW:
	{
		if (nowType == PROCVIEW_TYPE_WINDOW)
		{
			return;
		}
		nowType = PROCVIEW_TYPE_WINDOW;


		CString Temp;
		Temp.Format(L"���̴��� - %s", m_piProcInfo->Name);

		SetWindowText(Temp.GetBuffer());

		InitWindowList(&m_ListCtrl);

		//CloseHandle(CreateThread(NULL, 0,
		//	(LPTHREAD_START_ROUTINE)HsQueryProcessWindow,&m_viewList, 0, NULL));

		QueryProcessWindow(&m_ListCtrl);

		break;
	}
	
	case PROCVIEW_TYPE_MODULE:
	{
		if (nowType == PROCVIEW_TYPE_MODULE)
		{
			return;
		}
		nowType = PROCVIEW_TYPE_MODULE;

		CString Temp;
		Temp.Format(L"����ģ�� - %s", m_piProcInfo->Name);

		SetWindowText(Temp.GetBuffer());

		InitPModuleDetailList(&m_ListCtrl);

		LoadPModuleDetailList(&m_ListCtrl);

		break;
	}
	case PROCVIEW_TYPE_MEMORY:
	{
		if (nowType == PROCVIEW_TYPE_MEMORY)
		{
			return;
		}
		nowType = PROCVIEW_TYPE_MEMORY;

		CString Temp;
		Temp.Format(L"�����ڴ� - %s", m_piProcInfo->Name);

		SetWindowText(Temp.GetBuffer());

		InitMemoryList(&m_ListCtrl);

		QueryProcessMemory(&m_ListCtrl);

		break;
	}
	
	}

	m_ListCtrl.SetFocus();
}


void CProcessViewDlg::OnMenuProcessThreadRefresh()
{
	CloseHandle(CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)QueryProcessThread, &m_ListCtrl, 0, NULL));
}


void CProcessViewDlg::OnMenuProcessThreadClose()
{
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();

	while (pos)
	{
		UINT nItem = m_ListCtrl.GetNextSelectedItem(pos);

		ULONG ulThreadId = _ttoi(m_ListCtrl.GetItemText(nItem, 0));

		HANDLE hThread = OpenThread(THREAD_TERMINATE, FALSE, ulThreadId);

		BOOL bRet = TerminateThread(hThread, 0);

		if (bRet)
		{
			m_ListCtrl.DeleteItem(nItem);
		}
	}
}
