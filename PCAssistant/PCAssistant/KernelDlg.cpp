// KernelDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCAssistant.h"
#include "KernelDlg.h"
#include "afxdialogex.h"
#include "PCAssistantDlg.h"
#include "SSDTFunc.h"

// CKernelDlg �Ի���

enum KERNEL_LIST
{
	KERNEL_SSDT,
	KERNEL_SSSDT,
	KERNEL_KRNLFUNC,
	KERNEL_KRNLIAT,
	KERNEL_KRNLEAT,
};

BOOL bNowKernelSel = 255;

extern BOOL bIsChecking;	//��ǰ�ļ��״̬

IMPLEMENT_DYNAMIC(CKernelDlg, CDialogEx)

CKernelDlg::CKernelDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_KERNEL, pParent)
{
	m_wParent = pParent;
}

CKernelDlg::~CKernelDlg()
{
}

void CKernelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_KERNEL, m_ListKernel);
	DDX_Control(pDX, IDC_LIST_KERNELNAME, m_KernelNameList);
	DDX_Control(pDX, IDC_LIST_KERNEL_CTRL, m_ListKernelCtrl);
}


BEGIN_MESSAGE_MAP(CKernelDlg, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	ON_LBN_SELCHANGE(IDC_LIST_KERNEL, &CKernelDlg::OnLbnSelchangeListKernel)
END_MESSAGE_MAP()


// CKernelDlg ��Ϣ�������


BOOL CKernelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	InitKernelList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CKernelDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: �ڴ˴������Ϣ����������
	if (bShow == TRUE)
	{
		SendStatusDetail(L"����ϵͳ�ں���Ϣ��");
		SendStatusTip(L"�ں�");

		m_ListKernel.SetCurSel(KERNEL_SSDT);

		bNowKernelSel = 255;

		OnLbnSelchangeListKernel();

		m_ListKernelCtrl.SetFocus();

	}
}


void CKernelDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: �ڴ˴������Ϣ����������
					   // ��Ϊ��ͼ��Ϣ���� CDialogEx::OnPaint()
	CRect   rect;
	CRect KernelListRect;
	CRect KernelNameRect;
	CRect KernelCtrlRect;

	//m_ListKernel.GetClientRect(KernelListRect);
	m_ListKernel.GetWindowRect(&KernelListRect);
	ClientToScreen(&rect);
	KernelListRect.left -= rect.left;
	KernelListRect.right -= rect.left;
	KernelListRect.top -= rect.top;
	KernelListRect.bottom -= rect.top;
	KernelListRect.bottom = rect.Height() - 2;
	m_ListKernel.MoveWindow(KernelListRect);

	CPoint startPoint;
	startPoint.x = (LONG)(KernelListRect.right) + 2;
	startPoint.y = -1;
	CPoint endPoint;
	endPoint.x = (LONG)(KernelListRect.right) + 2;
	endPoint.y = rect.Height() + 2;

	KernelCtrlRect.left = startPoint.x + 1;
	KernelCtrlRect.right = rect.Width();
	KernelCtrlRect.top = 0;
	KernelCtrlRect.bottom = rect.Height();
	m_ListKernelCtrl.MoveWindow(KernelCtrlRect);
}

void CKernelDlg::InitKernelList(void)
{
	m_ListKernel.AddString(L"SSDT");
	m_ListKernel.InsertString(KERNEL_SSSDT, L"ShadowSSDT");
	m_ListKernel.InsertString(KERNEL_KRNLFUNC, L"�ں˺���");
	m_ListKernel.InsertString(KERNEL_KRNLIAT, L"�ں˵����");
	m_ListKernel.InsertString(KERNEL_KRNLEAT, L"�ں˵�����");

	m_ListKernel.SetItemHeight(-1, (UINT)16);
	m_KernelNameList.SetItemHeight(-1, (UINT)16);
}

void CKernelDlg::OnLbnSelchangeListKernel()
{
	int nCurSel = m_ListKernel.GetCurSel();

	switch (nCurSel)
	{
	case KERNEL_SSDT:
	{
		if (bIsChecking == TRUE || bNowKernelSel == KERNEL_SSDT)	//
		{
			m_ListKernel.SetCurSel(bNowKernelSel);
			m_ListKernelCtrl.SetFocus();
			return;
		}

		bNowKernelSel = nCurSel;
		bIsChecking = TRUE;

		m_KernelNameList.ShowWindow(FALSE);

		InitSSDTList(&m_ListKernelCtrl);

		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)LoadSSDTList, &m_ListKernelCtrl, 0, NULL)
		);
		//HsLoadSSDTList(&m_ListKernelCtrl);
		break;
	}
	/*
	case KERNEL_SSSDT:
	{
		if (bIsChecking == TRUE || bNowKernelSel == KERNEL_SSSDT)	//
		{
			m_ListKernel.SetCurSel(bNowKernelSel);
			m_ListKernelCtrl.SetFocus();
			return;
		}

		bNowKernelSel = nCurSel;
		bIsChecking = TRUE;

		m_KernelNameList.ShowWindow(FALSE);

		HsInitSSSDTList(&m_ListKernelCtrl);

		CloseHandle(
			CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)HsLoadSSSDTList, &m_ListKernelCtrl, 0, NULL)
		);
		//HsLoadSSSDTList(&m_ListKernelCtrl);
		break;
	}

	case KERNEL_KRNLFUNC:
	{
		if (bIsChecking == TRUE || bNowKernelSel == KERNEL_KRNLFUNC)	//
		{
			m_ListKernel.SetCurSel(bNowKernelSel);
			m_ListKernelCtrl.SetFocus();
			Sleep(50);
			return;
		}

		bNowKernelSel = nCurSel;

		m_KernelNameList.ShowWindow(TRUE);

		HsSendStatusDetail(L"�ں˺������ڼ���...");

		HsInitKernelFuncList(&m_KernelNameList, &m_ListKernelCtrl);

		m_KernelNameList.SetCurSel(0);

		OnLbnSelchangeListKrnlname();

		break;
	}

	case KERNEL_KRNLIAT:
	{
		if (bIsChecking == TRUE || bNowKernelSel == KERNEL_KRNLIAT)	//
		{
			m_ListKernel.SetCurSel(bNowKernelSel);
			m_ListKernelCtrl.SetFocus();
			Sleep(50);
			return;
		}

		bNowKernelSel = nCurSel;

		m_KernelNameList.ShowWindow(TRUE);

		HsSendStatusDetail(L"�ں˵�������ڼ���...");

		HsInitKernelFileList(&m_KernelNameList, &m_ListKernelCtrl);

		// 			m_KernelNameList.SetCurSel(0);
		// 
		// 			OnLbnSelchangeListKrnlname();

		break;
	}
	case KERNEL_KRNLEAT:
	{
		if (bIsChecking == TRUE || bNowKernelSel == KERNEL_KRNLEAT)	//
		{
			m_ListKernel.SetCurSel(bNowKernelSel);
			m_ListKernelCtrl.SetFocus();
			Sleep(50);
			return;
		}

		bNowKernelSel = nCurSel;

		m_KernelNameList.ShowWindow(TRUE);

		HsSendStatusDetail(L"�ں˵�������ڼ���...");

		HsInitKernelFileList(&m_KernelNameList, &m_ListKernelCtrl);

		// 			m_KernelNameList.SetCurSel(0);
		// 
		// 			OnLbnSelchangeListKrnlname();

		break;
	}
	default:
	{

	}
	*/
	}

	m_ListKernelCtrl.SetFocus();
}
