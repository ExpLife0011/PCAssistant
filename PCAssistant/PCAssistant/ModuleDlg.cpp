// ModuleDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCAssistant.h"
#include "ModuleDlg.h"
#include "afxdialogex.h"

#include "PCAssistantDlg.h"
#include "Common.h"
#include "KModuleFunc.h"

// CModuleDlg �Ի���
extern int ResizeX;
extern int ResizeY;

extern BOOL bIsChecking;

IMPLEMENT_DYNAMIC(CModuleDlg, CDialogEx)

CModuleDlg::CModuleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_MODULE, pParent)
{
	m_wParent = pParent;
}

CModuleDlg::~CModuleDlg()
{
}

void CModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MODULE, m_ListCtrl);
}


BEGIN_MESSAGE_MAP(CModuleDlg, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_MODULE, &CModuleDlg::OnNMRClickListModule)
	ON_COMMAND(ID_MENU_MODULE_REMOVE, &CModuleDlg::OnMenuModuleRemove)
	ON_COMMAND(ID_MENU_MODULE_REFRESH, &CModuleDlg::OnMenuModuleRefresh)
END_MESSAGE_MAP()


// CModuleDlg ��Ϣ�������


BOOL CModuleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitModuleList(&m_ListCtrl);
	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CModuleDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow == TRUE)
	{
		m_ListCtrl.MoveWindow(0, 0, ResizeX, ResizeY);

		LoadModuleList();

		SendStatusTip(L"����ģ��");

		m_ListCtrl.SetFocus();
	}
}


void CModuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	ResizeX = cx;
	ResizeY = cy;
}

VOID CModuleDlg::LoadModuleList()
{
	if (bIsChecking == TRUE)
	{
		return;
	}

	bIsChecking = TRUE;

	m_ListCtrl.DeleteAllItems();

	m_ListCtrl.SetSelectedColumn(-1);

	SendStatusDetail(L"ģ���б����ڼ���");

	CloseHandle(
		CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)QueryModuleFunction, &m_ListCtrl, 0, NULL)
	);

	return ;
}




void CModuleDlg::OnNMRClickListModule(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	CMenu	popup;
	CPoint	p;

	popup.LoadMenu(IDR_MENU_MODULE);				//���ز˵���Դ
	CMenu*	pM = popup.GetSubMenu(0);				//��ò˵�������

	GetCursorPos(&p);
	int	count = pM->GetMenuItemCount();
	if (m_ListCtrl.GetSelectedCount() == 0)		//���û��ѡ��
	{
		for (int i = 0; i<count; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //�˵�ȫ�����
		}
	}

	int Index = m_ListCtrl.GetSelectionMark();

	if (Index >= 0)
	{
		if (_wcsicmp(L"-", m_ListCtrl.GetItemText(Index, 3)) == 0)
		{
			pM->EnableMenuItem(ID_MENU_MODULE_REMOVE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
	}

	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);

	*pResult = 0;
}


void CModuleDlg::OnMenuModuleRemove()
{
	BOOL bRet = MessageBox(L"ж��ģ����������ܻ����ϵͳ�쳣�������\r\n����ȷ�Ϻ������", L"��Ӱ��ʿ", MB_ICONWARNING | MB_OKCANCEL);

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
		(LPTHREAD_START_ROUTINE)RemoveDriverModule, &m_ListCtrl, 0, NULL)
	);
}


void CModuleDlg::OnMenuModuleRefresh()
{
	LoadModuleList();
}
