// SystemDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCAssistant.h"
#include "SystemDlg.h"
#include "afxdialogex.h"
#include "PCAssistantDlg.h"
#include "Common.h"
#include "CallbackFunc.h"
#include "FilterDriverFunc.h"
#include "SysThread.h"

// CSystemDlg �Ի���
enum SYSK_LIST
{
	SYSK_CALLBACK,
	SYSK_FILTERDRIVER,
	SYSK_SYSTHREAD,
};

extern BOOL bIsChecking;

BOOL bNowSystemSel = 255;

extern int ResizeX;
extern int ResizeY;

IMPLEMENT_DYNAMIC(CSystemDlg, CDialogEx)

CSystemDlg::CSystemDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_SYSTEM, pParent)
{

}

CSystemDlg::~CSystemDlg()
{
}

void CSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SYSTEM, m_ListSystem);
	DDX_Control(pDX, IDC_LIST_SYSTEM_CTRL, m_ListSystemCtrl);
}


BEGIN_MESSAGE_MAP(CSystemDlg, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_LBN_SELCHANGE(IDC_LIST_SYSTEM, &CSystemDlg::OnLbnSelchangeListSystem)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_COMMAND(ID_MENU_CALLBACK_REFRESH, &CSystemDlg::OnMenuCallbackRefresh)
	ON_COMMAND(ID_MENU_CALLBACK_DELETE, &CSystemDlg::OnMenuCallbackDelete)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SYSTEM_CTRL, &CSystemDlg::OnNMRClickListSystemCtrl)
	ON_COMMAND(ID_MENU_FILTER_REFRESH, &CSystemDlg::OnMenuFilterRefresh)
	ON_COMMAND(ID_MENU_FILTER_REMOVE, &CSystemDlg::OnMenuFilterRemove)
END_MESSAGE_MAP()


// CSystemDlg ��Ϣ�������


BOOL CSystemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	InitSystemList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CSystemDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: �ڴ˴������Ϣ����������
	if (bShow == TRUE)
	{

		SendStatusDetail(L"��ʾ����ϵͳ�ں������Ϣ��");
		SendStatusTip(L"�ں�");

		m_ListSystem.SetCurSel(0);

		bNowSystemSel = 255;

		m_ListSystem.MoveWindow(0, 5, 99, 180);
		m_ListSystemCtrl.MoveWindow(100, 0, ResizeX, ResizeY);

		OnLbnSelchangeListSystem();

		m_ListSystemCtrl.SetFocus();
	}
}


void CSystemDlg::OnLbnSelchangeListSystem()
{
	int nCurSel = m_ListSystem.GetCurSel();

	switch (nCurSel)
	{
	case SYSK_CALLBACK:
	{
		if (bIsChecking == TRUE || bNowSystemSel == SYSK_CALLBACK)	//
		{
			m_ListSystem.SetCurSel(bNowSystemSel);
			m_ListSystemCtrl.SetFocus();
			return;
		}

		bNowSystemSel = nCurSel;

		InitCallBackList(&m_ListSystemCtrl);

		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)LoadCallBackList, &m_ListSystemCtrl, 0, NULL)
		);

		break;
	}
	
	case SYSK_FILTERDRIVER:
	{
		if (bIsChecking == TRUE || bNowSystemSel == SYSK_FILTERDRIVER)	//
		{
			m_ListSystem.SetCurSel(bNowSystemSel);
			m_ListSystemCtrl.SetFocus();
			return;
		}

		bNowSystemSel = nCurSel;

		InitFilterDriverList(&m_ListSystemCtrl);

		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)LoadFilterDriverList, &m_ListSystemCtrl, 0, NULL)
		);

		break;
	}
	
	case SYSK_SYSTHREAD:
	{
		if (bIsChecking == TRUE || bNowSystemSel == SYSK_SYSTHREAD)	//
		{
			m_ListSystem.SetCurSel(bNowSystemSel);
			m_ListSystemCtrl.SetFocus();
			return;
		}

		bNowSystemSel = nCurSel;

		InitSysThreadList(&m_ListSystemCtrl);

		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)LoadSysThreadList, &m_ListSystemCtrl, 0, NULL)
		);

		break;
	}
	default:
	{

	}
	
	}

	m_ListSystemCtrl.SetFocus();
}

void CSystemDlg::InitSystemList(void)
{
	m_ListSystem.AddString(L"ϵͳ�ص�");
	m_ListSystem.InsertString(SYSK_FILTERDRIVER, L"��������");
	m_ListSystem.InsertString(SYSK_SYSTHREAD, L"�ں��߳�");

	m_ListSystem.SetItemHeight(-1, 16);
}

void CSystemDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: �ڴ˴������Ϣ����������
					   // ��Ϊ��ͼ��Ϣ���� CDialogEx::OnPaint()
}


void CSystemDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	ResizeX = cx;
	ResizeY = cy;
}


void CSystemDlg::OnMenuCallbackRefresh()
{
	m_ListSystem.SetCurSel(SYSK_CALLBACK);
	bNowSystemSel = 255;
	OnLbnSelchangeListSystem();
}


void CSystemDlg::OnMenuCallbackDelete()
{
	if (bIsChecking)
	{
		return;
	}

	bIsChecking = TRUE;

	CloseHandle(
		CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)RemoveCallBackItem, &m_ListSystemCtrl, 0, NULL)
	);
}


void CSystemDlg::OnNMRClickListSystemCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	CMenu	popup;
	CPoint	p;

	switch (bNowSystemSel)
	{
	case SYSK_CALLBACK:
	{
		popup.LoadMenu(IDR_MENU_SYS_CALLBACK);			//���ز˵���Դ
		CMenu*	pM = popup.GetSubMenu(0);				//��ò˵�������

		GetCursorPos(&p);
		int	count = pM->GetMenuItemCount();
		if (m_ListSystemCtrl.GetSelectedCount() == 0)		//���û��ѡ��
		{
			for (int i = 1; i<count; i++)
			{
				pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //�˵�ȫ�����
			}
		}

		pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
		break;
	}
	case SYSK_FILTERDRIVER:
	{
		popup.LoadMenu(IDR_MENU_SYS_FILTER);		//���ز˵���Դ
		CMenu*	pM = popup.GetSubMenu(0);				//��ò˵�������

		GetCursorPos(&p);
		int	count = pM->GetMenuItemCount();
		if (m_ListSystemCtrl.GetSelectedCount() == 0)		//���û��ѡ��
		{
			for (int i = 1; i<count; i++)
			{
				pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //�˵�ȫ�����
			}
		}

		pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
		break;
	}

	}

	*pResult = 0;
}


void CSystemDlg::OnMenuFilterRefresh()
{
	m_ListSystem.SetCurSel(SYSK_FILTERDRIVER);
	bNowSystemSel = 255;
	OnLbnSelchangeListSystem();
}


void CSystemDlg::OnMenuFilterRemove()
{
	BOOL bRet = MessageBox(L"ж�ع��������������ܻ����ϵͳ�쳣�������\r\n����ȷ�Ϻ������", L"��Ӱ��ʿ", MB_ICONWARNING | MB_OKCANCEL);

	if (bRet == IDCANCEL)
	{
		return;
	}

	if (bIsChecking == TRUE)
	{
		return;
	}

	bIsChecking = TRUE;

	CloseHandle(
		CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)RemoveFilterDriverItem, &m_ListSystemCtrl, 0, NULL)
	);
}
