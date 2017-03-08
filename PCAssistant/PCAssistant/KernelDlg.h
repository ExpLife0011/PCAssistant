#pragma once
#include "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\atlmfc\include\afxwin.h"
#include "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\atlmfc\include\afxcmn.h"
#include "Common.h"

// CKernelDlg �Ի���

class CKernelDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CKernelDlg)

public:
	CKernelDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CKernelDlg();
	CWnd* m_wParent;
// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_KERNEL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_ListKernel;
	CListBox m_KernelNameList;
	CListCtrl m_ListKernelCtrl;
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	void InitKernelList(void);
	afx_msg void OnLbnSelchangeListKernel();
};
