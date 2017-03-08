#pragma once
#include "stdafx.h"
#include <WinIoCtl.h>
#include <winsvc.h>

typedef struct _COLUMN_STRUCT
{
	WCHAR*	szTitle;
	UINT    nWidth;
}COLUMN_STRUCT;

enum ENUM_DLG_MSG	//�ӶԻ������Ի���֮��ͨ��ö��
{
	MESSAGE_STATUSDETAIL = 5000,	//�޸�״̬����ϸ��Ϣ
	MESSAGE_STATUSTIP
};

enum HS_ENUM_IOCTL
{
	IOCTL_PROC = 0x100,					//�������
	IOCTL_PROC_SENDSELFPID,				//�����Լ���PID��������
	IOCTL_PROC_PROCESSCOUNT,			//���̼���
	IOCTL_PROC_PROCESSLIST,				//�оٽ����б�
	IOCTL_PROC_PROTECTPROCESS,			//��������
	IOCTL_PROC_KILLPROCESSBYFORCE,		//ǿ�ƹرս���
	IOCTL_PROC_PROCESSTHREAD,			//�߳�
	IOCTL_PROC_PROCESSTHREADMODULE,		//�߳�ģ��
	IOCTL_PROC_PROCESSPRIVILEGE,			//�оٽ���Ȩ��
	IOCTL_PROC_PRIVILEGE_ADJUST,			//�ı����Ȩ��
	IOCTL_PROC_PROCESSHANDLE,			//���
	IOCTL_PROC_PROCESSWINDOW,			//����
	IOCTL_PROC_PROCESSMODULE,			//����ģ��
	IOCTL_PROC_PROCESSMEMORY,			//�����ڴ�

	IOCTL_MODU = 0x180,					//ģ�����
	IOCTL_MODU_MODULELIST,				//�о�ϵͳģ���б�
	IOCTL_MODU_REMOVEMODULE,				//ж��ϵͳģ��

	IOCTL_KRNL = 0x200,					//�ں˹������
	IOCTL_KRNL_SSDTLIST,					//�о�SSDT�б�
	IOCTL_KRNL_KISRVTAB,					//��ȡ�����
	IOCTL_KRNL_RESUMESSDT,				//�ָ�SSDT����
	IOCTL_KRNL_SSSDTLIST,				//�о�SSSDT�б�
	IOCTL_KRNL_WIN32KSERVICE,			//��ȡWin32k�����
	IOCTL_KRNL_KRNLFILE,					//��ȡ�ں��ļ�		//�����ĸ��ļ���InputBuffer�Ĳ�������ȷ��
	IOCTL_KRNL_KRNLIAT,					//��ȡ�ں˵����
	IOCTL_KRNL_KRNLEAT,					//��ȡ�ں˵�����

	IOCTL_SYSK = 0x280,					//�ں����
	IOCTL_SYSK_SYSTHREAD,				//�ں��߳�
	IOCTL_SYSK_IOTIMER,					//IOTIMER
	IOCTL_SYSK_OPERIOTIMER,				//IOTIMER�л�
	IOCTL_SYSK_REMOVEIOTIMER,			//IOTIMER�Ƴ�
	IOCTL_SYSK_CALLBACKLIST,				//ϵͳ�ص�����������
	IOCTL_SYSK_REMOVECALLBACK,			//ɾ��ϵͳ�ص�
	IOCTL_SYSK_DPCTIMER,					//DPCTimer
	IOCTL_SYSK_REMOVEDPCTIMER,			//DPCTimerɾ��
	IOCTL_SYSK_FILTERDRIVER,				//��������
	IOCTL_SYSK_FILTERUNLOAD,				//ж�ع�������
};

typedef enum _WIN_VERSION
{
	WindowsNT,
	Windows2000,
	WindowsXP,
	Windows2003,
	WindowsVista,
	Windows7,
	Windows8,
	Windows8_1,
	Windows10,
	WinUnknown
}WIN_VERSION;

enum DLG_NUM
{
	DIALOG_PROCESS = 0,
	DIALOG_MOUDLE,
	DIALOG_SYSTEM,
	DIALOG_TIME,

};

#define DEVICE_NAME			L"\\Device\\PCAssistantDevice"
#define LINK_NAME			L"\\\\.\\PCAssistantLink"

#define IOCTL(i)			 \
	CTL_CODE                 \
	(                        \
	FILE_DEVICE_UNKNOWN,     \
	i,						 \
	METHOD_NEITHER,          \
	FILE_ANY_ACCESS          \
	)

BOOL Is64BitWindows();
BOOL LoadNTDriver(WCHAR* lpszDriverName, WCHAR* lpszDriverPath);
BOOL UnloadNTDriver(WCHAR* szSvrName);
VOID SendStatusDetail(LPCWSTR szBuffer);
VOID SendStatusTip(LPCWSTR szBuffer);
WIN_VERSION  GetWindowsVersion();
CString TrimPath(WCHAR * wzPath);
CString GetLongPath(CString szPath);
ULONG_PTR GetKernelBase(char* szNtosName);
char *Strcat(char *Str1, char *Str2);
CHAR* GetSystemDir();
DWORD FileLen(char* szFileName);
CHAR* LoadDllContext(char* szFileName);
int Reloc(ULONG_PTR NewBase, ULONG_PTR OrigBase);
CHAR *GetTempNtdll();
DWORD GetSpecialIndex(char *FunctionName);
