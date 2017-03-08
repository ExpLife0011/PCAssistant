#include "stdafx.h"
#include "CallbackFunc.h"
#include "KModuleFunc.h"
#include "SSDTFunc.h"
#include "ProcessFunc.h"
#include "Common.h"
#include <vector>

using namespace std;

extern HANDLE g_hDevice;
extern WIN_VERSION WinVersion;
extern BOOL bIsChecking;

ULONG_PTR m_ulAllNotify = 0;

ULONG_PTR m_ulNotifyLoadImage = 0;
ULONG_PTR m_ulNotifyCmpCallback = 0;
ULONG_PTR m_ulNotifyCreateProcess = 0;
ULONG_PTR m_ulNotifyCreateThread = 0;
ULONG_PTR m_ulNotifyShutdown = 0;
ULONG_PTR m_ulNotifyCheckReason = 0;
ULONG_PTR m_ulNotifyCheck = 0;

extern vector<DRIVER_INFO> m_DriverList;
vector<CALLBACK_INFO> m_CallbackVector;

COLUMN_STRUCT g_Column_CallBack[] =
{
	{ L"�ص���ַ",				125 },
	{ L"�ص�����",				125 },
	{ L"ģ���ļ�",				225 },
	{ L"��Ʒ����",				125 },
	{ L"����",					125 },
};

UINT g_Column_CallBack_Count = 5;

VOID InitCallBackList(CListCtrl *m_ListCtrl)
{
	while (m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0; i<g_Column_CallBack_Count; i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_CallBack[i].szTitle, LVCFMT_LEFT, (int)(g_Column_CallBack[i].nWidth));
	}
}

VOID LoadCallBackList(CListCtrl *m_ListCtrl)
{
	if (bIsChecking == TRUE)
	{
		return;
	}

	bIsChecking = TRUE;

	SendStatusDetail(L"ϵͳ�ص����ڼ���...");
	SendStatusTip(L"ϵͳ�ص�");

	QueryCallBackList(m_ListCtrl);

	bIsChecking = FALSE;
}

VOID QueryCallBackList(CListCtrl *m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();
	m_CallbackVector.clear();

	EnumDriver();

	if (QueryCallBackList() == FALSE)
	{
		SendStatusDetail(L"ϵͳ�ص�����ʧ�ܡ�");
		return;
	}

	InsertCallbackItem(m_ListCtrl);
}

BOOL QueryCallBackList()
{
	ULONG_PTR ulCnt = 100;
	PGET_CALLBACK CallbackInfor = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	do
	{
		ULONG_PTR ulSize = sizeof(GET_CALLBACK) + ulCnt * sizeof(CALLBACK_INFO);

		if (CallbackInfor)
		{
			free(CallbackInfor);
			CallbackInfor = NULL;
		}

		CallbackInfor = (PGET_CALLBACK)malloc(ulSize);

		if (CallbackInfor)
		{
			memset(CallbackInfor, 0, ulSize);
			CallbackInfor->ulCnt = ulCnt;

			BOOL NotifyType = NotifyLoadImage;

			bRet = DeviceIoControl(g_hDevice, IOCTL(IOCTL_SYSK_CALLBACKLIST),
				&NotifyType,
				sizeof(BOOL),
				CallbackInfor,
				(DWORD)ulSize,
				&ulReturnSize,
				NULL);
		}

		ulCnt = CallbackInfor->ulCnt + 10;

	} while (!bRet && CallbackInfor->ulRetCnt > CallbackInfor->ulCnt);

	if (bRet &&
		CallbackInfor->ulCnt >= CallbackInfor->ulRetCnt)
	{
		for (ULONG i = 0; i < CallbackInfor->ulRetCnt; i++)
		{
			m_CallbackVector.push_back(CallbackInfor->Callbacks[i]);
		}
	}

	if (CallbackInfor)
	{
		free(CallbackInfor);
		CallbackInfor = NULL;
	}

	return bRet;
}


VOID InsertCallbackItem(CListCtrl* m_ListCtrl)
{
	m_ulNotifyLoadImage = 0;
	m_ulNotifyCmpCallback = 0;
	m_ulNotifyCreateProcess = 0;
	m_ulNotifyCreateThread = 0;
	m_ulNotifyShutdown = 0;
	m_ulNotifyCheckReason = 0;
	m_ulNotifyCheck = 0;

	m_ulAllNotify = 0;

	for (vector<CALLBACK_INFO>::iterator itor = m_CallbackVector.begin(); itor != m_CallbackVector.end(); itor++)
	{
		CString strCallbackAddress, strType, strPath, strComp, strNote;

		strCallbackAddress.Format(L"0x%p", itor->CallbackAddress);
		int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strCallbackAddress);


		strNote.Format(L"0x%p", itor->Note);


		switch (itor->Type)
		{
		case NotifyLoadImage:
		{
			strType = L"LoadImage";

			m_ulNotifyLoadImage++;
			break;
		}
		case NotifyCmCallBack:
		{
			strType = L"CmpCallBack";

			m_ulNotifyCmpCallback++;
			break;
		}
		case NotifyCreateProcess:
		{
			strType = L"CreateProcess";

			m_ulNotifyCreateProcess++;
			break;
		}
		case NotifyCreateThread:
		{
			strType = L"CreateThread";

			m_ulNotifyCreateThread++;
			break;
		}
		case NotifyShutdown:
		{
			strType = L"Shutdown";

			m_ulNotifyShutdown++;
			break;
		}
		case NotifyKeBugCheckReason:
		{
			strType = L"BugCheckReason";

			m_ulNotifyCheckReason++;
			break;
		}
		case NotifyKeBugCheck:
		{
			strType = L"BugCheck";

			m_ulNotifyCheck++;
			break;
		}
		default:
		{
			break;
		}
		}
		m_ulAllNotify++;



		strPath = GetDriverPath(itor->CallbackAddress);
		strComp = GetFileCompanyName(strPath);

		m_ListCtrl->SetItemText(n, 1, strType);
		m_ListCtrl->SetItemText(n, 2, strPath);
		m_ListCtrl->SetItemText(n, 3, strComp);
		m_ListCtrl->SetItemText(n, 4, strNote);


		CString StatusBarContext;
		StatusBarContext.Format(
			L"ϵͳ�ص����ڼ��ء� �������أ�%d��ע���%d�����̴�����%d���̴߳�����%d���ػ���%d��������%d",
			m_ulNotifyLoadImage,
			m_ulNotifyCmpCallback,
			m_ulNotifyCreateProcess,
			m_ulNotifyCreateThread,
			m_ulNotifyShutdown,
			m_ulNotifyCheckReason + m_ulNotifyCheck);

		SendStatusDetail(StatusBarContext);

	}

	CString StatusBarContext;
	StatusBarContext.Format(
		L"ϵͳ�ص�������ɡ� �������أ�%d��ע���%d�����̴�����%d���̴߳�����%d���ػ���%d��������%d",
		m_ulNotifyLoadImage,
		m_ulNotifyCmpCallback,
		m_ulNotifyCreateProcess,
		m_ulNotifyCreateThread,
		m_ulNotifyShutdown,
		m_ulNotifyCheckReason + m_ulNotifyCheck);

	SendStatusDetail(StatusBarContext);

}

VOID RemoveCallBackItem(CListCtrl* m_ListCtrl)
{
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;
	int Index = m_ListCtrl->GetSelectionMark();

	if (Index<0)
	{
		return;
	}


	CString Temp = m_ListCtrl->GetItemText(Index, 0);

	REMOVE_CALLBACK  RemoveCallBack;

	for (vector <CALLBACK_INFO>::iterator Iter = m_CallbackVector.begin(); Iter != m_CallbackVector.end(); Iter++)
	{
		CString strCallback;
		strCallback.Format(L"0x%p", Iter->CallbackAddress);
		if (!strCallback.CompareNoCase(Temp))
		{

			RemoveCallBack.CallbackAddress = Iter->CallbackAddress;
			RemoveCallBack.Note = Iter->Note;
			RemoveCallBack.NotifyType = Iter->Type;

			bRet = DeviceIoControl(g_hDevice, IOCTL(IOCTL_SYSK_REMOVECALLBACK),
				&RemoveCallBack,
				sizeof(REMOVE_CALLBACK),
				NULL,
				0,
				&ulReturnSize,
				NULL);


			break;
		}
	}

	if (bRet)
	{
		m_ulNotifyLoadImage--;
		m_ulAllNotify--;
		m_ListCtrl->DeleteItem(Index);

		CString StatusBarContext;
		StatusBarContext.Format(
			L"ϵͳ�ص�������ɡ� �������أ�%d��ע���%d�����̴�����%d���̴߳�����%d���ػ���%d��������%d",
			m_ulNotifyLoadImage,
			m_ulNotifyCmpCallback,
			m_ulNotifyCreateProcess,
			m_ulNotifyCreateThread,
			m_ulNotifyShutdown,
			m_ulNotifyCheckReason + m_ulNotifyCheck);

		SendStatusDetail(StatusBarContext);
	}


	bIsChecking = FALSE;
}