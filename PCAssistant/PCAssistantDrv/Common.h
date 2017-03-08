#pragma once 

#include <ntifs.h>

#define DEVICE_NAME                  L"\\Device\\PCAssistantDevice"
#define LINK_NAME                    L"\\DosDevices\\PCAssistantLink"

enum ENUM_IOCTL
{
	IOCTL_PROC = 0x100,				//�������
	IOCTL_PROC_SENDSELFPID,			//�����Լ���PID��������
	IOCTL_PROC_PROCESSCOUNT,				//���̼���
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

typedef
NTSTATUS
(*pfnRtlGetVersion)(OUT PRTL_OSVERSIONINFOW lpVersionInformation);

typedef enum WIN_VERSION {
	WINDOWS_UNKNOW,
	WINDOWS_XP,
	WINDOWS_7,
	WINDOWS_8,
	WINDOWS_8_1
} WIN_VERSION;

WIN_VERSION GetWindowsVersion();
PVOID GetFunctionAddressByName(WCHAR *szFunction);
BOOLEAN IsUnicodeStringValid(PUNICODE_STRING uniString);