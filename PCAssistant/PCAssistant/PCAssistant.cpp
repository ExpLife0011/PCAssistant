
// PCAssistant.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "PCAssistant.h"
#include "PCAssistantDlg.h"
#include "Common.h"

extern HANDLE g_hDevice;
BOOL bDriverIsOK = FALSE;



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPCAssistantApp

BEGIN_MESSAGE_MAP(CPCAssistantApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPCAssistantApp ����

CPCAssistantApp::CPCAssistantApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CPCAssistantApp ����

CPCAssistantApp theApp;


// CPCAssistantApp ��ʼ��

BOOL CPCAssistantApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	HANDLE hMutex = ::CreateMutex(NULL, TRUE, L"PA_ONLE_ONE_PROCESS");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		::MessageBox(NULL, L"����������PCС����",L"Notice", MB_ICONERROR);

		CloseHandle(hMutex);
		ExitProcess(0);
	}

	if (Is64BitWindows() == TRUE && sizeof(ULONG_PTR) == sizeof(ULONG32))
	{
		::MessageBox(NULL, L"������ʹ��64λ����ϵͳ��������64λPCС����", L"Notice", MB_ICONERROR);

		ExitProcess(0);
	}
	
	
	
	WCHAR wzSysPath[MAX_PATH] = { 0 };
	WCHAR *p;
	CString SysPath;
	HMODULE hModule = GetModuleHandle(0);
	GetModuleFileName(hModule, wzSysPath, sizeof(wzSysPath));
	p = wcsrchr(wzSysPath, L'\\');
	*p = 0;
	SysPath = wzSysPath;

	if (Is64BitWindows())
	{
		SysPath += L"\\sys\\PCAssistantSys64.sys";
	}
	else
	{
		SysPath += L"\\sys\\PCAssistantSys32.sys";
	}

	LoadNTDriver(L"PCAssistantSys", SysPath.GetBuffer());

	g_hDevice = CreateFileW(LINK_NAME,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);


	if (g_hDevice != INVALID_HANDLE_VALUE)
	{
		ULONG_PTR ulCurrentPid = GetCurrentProcessId();
		ULONG_PTR ulRetCode = 0;
		DWORD dwReturnSize = 0;

		BOOL dwRet = DeviceIoControl(g_hDevice,
			IOCTL(IOCTL_PROC_SENDSELFPID),
			&ulCurrentPid,
			sizeof(ULONG_PTR),
			&ulRetCode,
			sizeof(ULONG_PTR),
			&dwReturnSize,
			NULL);

		if (dwRet && ulRetCode)
		{
			bDriverIsOK = TRUE;
		}
	}

	

	CPCAssistantDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	CloseHandle(hMutex);
	CloseHandle(g_hDevice);
	UnloadNTDriver(L"PCAssistantSys");

	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

