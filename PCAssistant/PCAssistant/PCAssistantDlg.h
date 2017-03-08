
// PCAssistantDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "ProcessDlg.h"
#include "ModuleDlg.h"
#include "SystemDlg.h"
#include "TimeDlg.h"

// CPCAssistantDlg �Ի���
class CPCAssistantDlg : public CDialogEx
{
// ����
public:
	CPCAssistantDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PCASSISTANT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	CProcessDlg*	m_ProcessDlg;
	CModuleDlg*		m_ModuleDlg;
	CSystemDlg*		m_SystemDlg;
	CTimeDlg*		m_TimeDlg;
	LRESULT SetStatusDetail(WPARAM wParam, LPARAM lParam);
	LRESULT SetStatusTip(WPARAM wParam, LPARAM lParam);
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_MainTab;
	CStatusBarCtrl* m_StatusBar;

	VOID InitTab();
	afx_msg void OnTcnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult);
};
