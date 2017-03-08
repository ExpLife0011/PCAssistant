
// PCAssistantDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PCAssistant.h"
#include "PCAssistantDlg.h"
#include "afxdialogex.h"
#include "Common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern HANDLE g_hDevice;

CWnd* g_wParent = NULL;
BOOL bIsChecking = FALSE; //��ǰ�ļ��״̬

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPCAssistantDlg �Ի���



CPCAssistantDlg::CPCAssistantDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PCASSISTANT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_ProcessDlg = NULL;
	m_ModuleDlg	= NULL;
	m_SystemDlg = NULL;
	m_TimeDlg = NULL;
}

void CPCAssistantDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_MAIN, m_MainTab);
}



BEGIN_MESSAGE_MAP(CPCAssistantDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(MESSAGE_STATUSDETAIL, SetStatusDetail)
	ON_MESSAGE(MESSAGE_STATUSTIP, SetStatusTip)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, &CPCAssistantDlg::OnTcnSelchangeTabMain)
END_MESSAGE_MAP()


// CPCAssistantDlg ��Ϣ�������

BOOL CPCAssistantDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	g_wParent = this;

	//���״̬��
	m_StatusBar = new CStatusBarCtrl;

	m_StatusBar->Create(WS_CHILD | WS_VISIBLE | SBT_OWNERDRAW, CRect(0, 0, 0, 0), this, 0);

	int strPartDim[3] = { 745,845,0 };
	m_StatusBar->SetParts(3, strPartDim);

	m_StatusBar->SetText(L"׼��������", 0, 0);
	m_StatusBar->SetText(L"", 1, 0);

	InitTab();

	if (m_ProcessDlg == NULL)
	{
		m_ProcessDlg = new CProcessDlg(this);
		m_ProcessDlg->Create(IDD_DIALOG_PROCESS, GetDlgItem(IDC_TAB_MAIN));

		CRect Rect;
		m_MainTab.GetClientRect(&Rect);
		Rect.top += 21;
		Rect.left += 1;
		Rect.right -= 1;
		Rect.bottom -= 3;
		m_ProcessDlg->MoveWindow(&Rect);
	}

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CPCAssistantDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPCAssistantDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPCAssistantDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

VOID CPCAssistantDlg::InitTab()
{
	m_MainTab.InsertItem(0, L"����");
	m_MainTab.InsertItem(1, L"����ģ��");
	m_MainTab.InsertItem(2, L"�ں�");
	m_MainTab.InsertItem(3, L"��ʱ��");
	return;
}




void CPCAssistantDlg::OnTcnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int iSelect = m_MainTab.GetCurSel();

	switch (iSelect)
	{
	case DIALOG_PROCESS:
	{
		if (m_ProcessDlg == NULL)
		{
			m_ProcessDlg = new CProcessDlg(this);
			m_ProcessDlg->Create(IDD_DIALOG_PROCESS, GetDlgItem(IDC_TAB_MAIN));

			CRect Rect;
			m_MainTab.GetClientRect(&Rect);
			Rect.top += 21;
			Rect.left += 1;
			Rect.right -= 1;
			Rect.bottom -= 3;
			m_ProcessDlg->MoveWindow(&Rect);
		}
		if (m_ProcessDlg)
			m_ProcessDlg->ShowWindow(TRUE);
		if (m_ModuleDlg)
			m_ModuleDlg->ShowWindow(FALSE);
		if (m_SystemDlg)
			m_SystemDlg->ShowWindow(FALSE);

		break;
	}
	case DIALOG_MOUDLE:
	{
		if (m_ModuleDlg == NULL)
		{
			m_ModuleDlg = new CModuleDlg(this);
			m_ModuleDlg->Create(IDD_DIALOG_MODULE, GetDlgItem(IDC_TAB_MAIN));

			CRect Rect;
			m_MainTab.GetClientRect(&Rect);
			Rect.top += 21;
			Rect.left += 1;
			Rect.right -= 1;
			Rect.bottom -= 3;
			m_ModuleDlg->MoveWindow(&Rect);
		}
		if (m_ModuleDlg)
			m_ModuleDlg->ShowWindow(TRUE);
		if (m_ProcessDlg)
			m_ProcessDlg->ShowWindow(FALSE);
		if (m_SystemDlg)
			m_SystemDlg->ShowWindow(FALSE);
		if (m_TimeDlg)
			m_TimeDlg->ShowWindow(FALSE);
		break;
		
	}
	case DIALOG_SYSTEM:
	{
		if (m_SystemDlg == NULL)
		{
			m_SystemDlg = new CSystemDlg(this);
			m_SystemDlg->Create(IDD_DIALOG_SYSTEM, GetDlgItem(IDC_TAB_MAIN));

			CRect Rect;
			m_MainTab.GetClientRect(&Rect);
			Rect.top += 21;
			Rect.left += 1;
			Rect.right -= 1;
			Rect.bottom -= 3;
			m_SystemDlg->MoveWindow(&Rect);
		}
		if (m_SystemDlg)
			m_SystemDlg->ShowWindow(TRUE);
		if (m_ModuleDlg)
			m_ModuleDlg->ShowWindow(FALSE);
		if (m_ProcessDlg)
			m_ProcessDlg->ShowWindow(FALSE);
		if (m_TimeDlg)
			m_TimeDlg->ShowWindow(FALSE);

		break;
	}
	case DIALOG_TIME:
	{
		if (m_TimeDlg == NULL)
		{
			m_TimeDlg = new CTimeDlg(this);
			m_TimeDlg->Create(IDD_DIALOG_TIME, GetDlgItem(IDC_TAB_MAIN));

			CRect Rect;
			m_MainTab.GetClientRect(&Rect);
			Rect.top += 21;
			Rect.left += 1;
			Rect.right -= 1;
			Rect.bottom -= 3;
			m_TimeDlg->MoveWindow(&Rect);
		}
		if (m_TimeDlg)
			m_TimeDlg->ShowWindow(TRUE);
		if (m_SystemDlg)
			m_SystemDlg->ShowWindow(FALSE);
		if (m_ModuleDlg)
			m_ModuleDlg->ShowWindow(FALSE);
		if (m_ProcessDlg)
			m_ProcessDlg->ShowWindow(FALSE);
		

		break;
	}
	default:
		break;
	}

	*pResult = 0;
}


LRESULT CPCAssistantDlg::SetStatusDetail(WPARAM wParam, LPARAM lParam)
{
	CString recvstr = (LPCTSTR)lParam;

	m_StatusBar->SetText(recvstr.GetBuffer(), 0, 0);
	return TRUE;
}

LRESULT CPCAssistantDlg::SetStatusTip(WPARAM wParam, LPARAM lParam)
{
	CString recvstr = (LPCTSTR)lParam;

	m_StatusBar->SetText(recvstr.GetBuffer(), 1, 0);
	return TRUE;
}
