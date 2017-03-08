#include "Process.h"
#include "Privilege.h"
#include "Thread.h"
#include "Handle.h"
#include "Window.h"

ULONG_PTR ulCurrentProcessId = 1000000;				//��Ҫ

ULONG_PTR    ObjectTableOffsetOf_EPROCESS = 0;		//�����ƫ�ơ�
ULONG_PTR    PreviousModeOffsetOf_KTHREAD = 0;		//Ȩ����ص�ƫ�ơ�
ULONG_PTR    ImageFileNameOffset = 0;		//�ļ���ƫ�ơ�
ULONG_PTR    SectionObjectOffsetOfEProcess = 0;		//SectionObject
ULONG_PTR    ParentProcessIdOffset = 0;		//������IDƫ��InheritedFromUniqueProcessId

ULONG_PTR    ObjectHeaderSize = 0;
ULONG_PTR    ObjectTypeOffsetOf_Object_Header = 0;

extern WIN_VERSION  WinVersion;

NTSTATUS DispatchControlForProcess(PIO_STACK_LOCATION  IrpSp, PVOID OutputBuffer, ULONG_PTR* ulRet)
{
	PVOID               pvInputBuffer   = NULL;
	ULONG				ulIoControlCode = 0;
	ULONG               ulInputLen      = 0;
	ULONG				ulOutputLen		= 0;
	NTSTATUS			Status			= STATUS_UNSUCCESSFUL;

	pvInputBuffer   = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
	ulInputLen      = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	ProbeForRead(pvInputBuffer,ulInputLen,sizeof(CHAR));
	ulOutputLen = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	ProbeForWrite(OutputBuffer, ulOutputLen, sizeof(CHAR));
	ulIoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
	ulIoControlCode = (ulIoControlCode>>2)&0x00000FFF;

	InitProcessGlobalVariable();

	switch(ulIoControlCode)
	{
	case IOCTL_PROC_SENDSELFPID:
		{
			DbgPrint("IOCTL_PROC_SENDSELFPID\r\n");
			ulCurrentProcessId = *(ULONG_PTR*)pvInputBuffer;
			DbgPrint("CURRENTPID: %d\r\n",ulCurrentProcessId);
			*(ULONG_PTR*)OutputBuffer = 444;
			Status = STATUS_SUCCESS;
			break;
		}

	case IOCTL_PROC_PROCESSLIST:			//����
	{
		DbgPrint("IOCTL_PROC_PROCESSLIST\r\n");
		Status = EnumSystemProcessList(*(ULONG_PTR*)pvInputBuffer, (PPROCESSINFO)OutputBuffer, ulRet);
		break;
	}
	
	case IOCTL_PROC_KILLPROCESSBYFORCE:	//����ɱ������
	{
		DbgPrint("IOCTL_PROC_KILLPROCESSBYFORCE\r\n");
		Status = KillProcessByZeroMemory(*((ULONG_PTR*)pvInputBuffer), (int*)OutputBuffer);
		break;
	}

	case IOCTL_PROC_PROCESSTHREAD:		//�߳�
	{
		DbgPrint("IOCTL_PROC_PROCESSTHREAD\r\n");
		//Status = HsEnumProcessThread(pvInputBuffer);
		Status = EnumProcessThread(pvInputBuffer, ulInputLen, OutputBuffer, ulOutputLen);
		break;
	}

	case IOCTL_PROC_PROCESSTHREADMODULE://�߳�
	{
		DbgPrint("IOCTL_PROC_PROCESSTHREADMODULE\r\n");
		Status = EnumProcessThreadModule(*((ULONG_PTR*)pvInputBuffer), OutputBuffer, ulOutputLen);
		break;
	}

	case IOCTL_PROC_PROCESSPRIVILEGE:	//Ȩ��
	{
		DbgPrint("IOCTL_PROC_PROCESSPRIVILEGE\r\n");
		Status = EnumProcessPrivilege((WCHAR*)OutputBuffer, *(ULONG_PTR*)pvInputBuffer, *ulRet);
		break;
	}


	case IOCTL_PROC_PROCESSHANDLE:		//���̾��
	{
		DbgPrint("IOCTL_PROC_PROCESSHANDLE\r\n");
		Status = EnumProcessHandle((PVOID)*(PULONG)pvInputBuffer, ulInputLen, OutputBuffer, ulOutputLen);
		break;
	}

	case IOCTL_PROC_PROCESSWINDOW:		//����
	{
		DbgPrint("IOCTL_PROC_PROCESSWINDOW\r\n");
		Status = EnumProcessWindow(pvInputBuffer, ulInputLen, OutputBuffer, ulOutputLen);
		break;
	}

	case IOCTL_PROC_PROCESSMODULE:		//����ģ��
	{
		DbgPrint("IOCTL_PROC_PROCESSMODULE\r\n");
		Status = EnumProcessesModule(*((ULONG_PTR*)pvInputBuffer), OutputBuffer, ulOutputLen);
		break;
	}

	case IOCTL_PROC_PROCESSMEMORY:		//�����ڴ�
	{
		DbgPrint("IOCTL_PROC_PROCESSMEMORY\r\n");
		Status = EnumProcessesMemory(*((ULONG_PTR*)pvInputBuffer), OutputBuffer, ulOutputLen);
		break;
	}

	default:
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

	return Status;
}

NTSTATUS EnumSystemProcessList(ULONG_PTR ulBasePid, PPROCESSINFO plProcessList, ULONG_PTR* ulRet)
{
	NTSTATUS            Status = STATUS_UNSUCCESSFUL;
	ULONG_PTR           iPid = 0;
	OBJECT_ATTRIBUTES   oa = { 0 };
	CLIENT_ID           Cid = { 0 };
	HANDLE              hProcess = NULL;
	PEPROCESS           EProcess = NULL;

	WCHAR				szImageFilePath[260] = { 0 };

	ULONG_PTR	        ulParentPid = 0;

	PETHREAD EThread = NULL;
	CHAR     PreMode = 0;

	//////////////////////////////////////////////////////////////////////////

	EThread = PsGetCurrentThread();
	PreMode = ChangePreMode(EThread);
	
	
	
	for (iPid = ulBasePid; iPid < MAX_PROCESS_COUNT; iPid += 4)
	{
		Cid.UniqueProcess = (HANDLE)iPid;
		Cid.UniqueThread = 0;

		//ͨ��ID �õ� Handle

		if (!iPid)
		{
			plProcessList->Pid = iPid;
			plProcessList->PPid = 0;
			plProcessList->Eprocess = (ULONG_PTR)GetIdleEProcess();

			break;
		}

		if (ulCurrentProcessId == iPid)
		{
			EProcess = PsGetCurrentProcess();
			goto NEXT;
		}

		Status = ZwOpenProcess(&hProcess, GENERIC_ALL, &oa, &Cid);   //System   Session  Native API   

		if (NT_SUCCESS(Status))
		{
			DbgPrint("PID : %d\r\n", iPid);

			//ת�� Handle  �� EProcess
			
			ObReferenceObjectByHandle(hProcess, GENERIC_ALL, NULL, KernelMode, (PVOID*)&EProcess, NULL);

NEXT:
			if (IsRealProcess(EProcess))	//�ж��Ƿ�ʬ����
			{

				ulParentPid = GetParentProcessIdByEProcess(EProcess);

				plProcessList->Pid = iPid;

				plProcessList->PPid = ulParentPid;

				DbgPrint("NAME: %s\r\n", (char*)((ULONG_PTR)EProcess + ImageFileNameOffset));

				GetProcessPathBySectionObject(iPid, szImageFilePath);

				memcpy(plProcessList->Path, szImageFilePath, sizeof(szImageFilePath));

				memset(szImageFilePath, 0, sizeof(szImageFilePath));

				DbgPrint("EPRO: 0x%x\r\n", (ULONG_PTR)EProcess);

				plProcessList->Eprocess = (ULONG_PTR)EProcess;

				ObDereferenceObject(EProcess);

				ZwClose(hProcess);

				break;
			}

			ObDereferenceObject(EProcess);

			ZwClose(hProcess);

		}
		memset(&oa, 0, sizeof(OBJECT_ATTRIBUTES));
	}

	Status = STATUS_SUCCESS;

	if (iPid >= MAX_PROCESS_COUNT)
	{
		plProcessList->Eprocess = 0;
		plProcessList->PPid = 0;
		*ulRet = 0;
		Status = STATUS_UNSUCCESSFUL;
	}

	RecoverPreMode(EThread, PreMode);
	return Status;
	
}

VOID InitProcessGlobalVariable()
{
	switch (WinVersion)
	{
	case WINDOWS_7:
	{
		ImageFileNameOffset = 0x2e0;
		PreviousModeOffsetOf_KTHREAD = 0x1f6;
		ObjectTableOffsetOf_EPROCESS = 0x200;
		ParentProcessIdOffset = 0x290;
		break;
	}
	case WINDOWS_XP:
	{
		ImageFileNameOffset = 0x174;
		PreviousModeOffsetOf_KTHREAD = 0x140;
		ObjectTableOffsetOf_EPROCESS = 0x0c4;
		ParentProcessIdOffset = 0x14c;
		ObjectHeaderSize = 0x18;
		ObjectTypeOffsetOf_Object_Header = 0x8;
		break;
	}
	}
}


BOOLEAN IsRealProcess(PEPROCESS EProcess)
{
	ULONG_PTR ObjectType;
	ULONG_PTR    ObjectTypeAddress;
	BOOLEAN bRet = FALSE;

	ULONG_PTR ProcessType = ((ULONG_PTR)*PsProcessType);

	if (ProcessType && MmIsAddressValid && EProcess && MmIsAddressValid((PVOID)(EProcess)))
	{
		ObjectType = KeGetObjectType((PVOID)EProcess);
		if (ObjectType &&
			ProcessType == ObjectType &&
			!IsProcessDie(EProcess))
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

BOOLEAN IsProcessDie(PEPROCESS EProcess)
{
	BOOLEAN bDie = FALSE;

	if (MmIsAddressValid &&
		EProcess &&
		MmIsAddressValid(EProcess) &&
		MmIsAddressValid((PVOID)((ULONG_PTR)EProcess + ObjectTableOffsetOf_EPROCESS)))
	{
		PVOID ObjectTable = *(PVOID*)((ULONG_PTR)EProcess + ObjectTableOffsetOf_EPROCESS);

		if (!ObjectTable || !MmIsAddressValid(ObjectTable))
		{
			DbgPrint("Process is Die\r\n");
			bDie = TRUE;
		}
	}
	else
	{
		DbgPrint("Process is Die2\r\n");
		bDie = TRUE;
	}
	return bDie;
}


BOOLEAN GetProcessPathBySectionObject(ULONG_PTR ulProcessID, WCHAR* wzProcessPath)
{
	PEPROCESS         EProcess = NULL;
	PSECTION_OBJECT   SectionObject = NULL;
	PSECTION_OBJECT64 SectionObject64 = NULL;
	PSEGMENT        Segment = NULL;
	PSEGMENT64      Segment64 = NULL;
	PCONTROL_AREA   ControlArea = NULL;
	PCONTROL_AREA64 ControlArea64 = NULL;
	PFILE_OBJECT    FileObject = NULL;
	BOOLEAN         bGetPath = FALSE;

	if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)ulProcessID, &EProcess)))
	{

		switch (WinVersion)
		{
		case WINDOWS_XP:
		{
			SectionObjectOffsetOfEProcess = 0x138;

			if (SectionObjectOffsetOfEProcess != 0 && MmIsAddressValid((PVOID)((ULONG_PTR)EProcess + SectionObjectOffsetOfEProcess)))
			{
				SectionObject = *(PSECTION_OBJECT*)((ULONG_PTR)EProcess + SectionObjectOffsetOfEProcess);

				if (SectionObject && MmIsAddressValid(SectionObject))
				{

					Segment = (PSEGMENT)SectionObject->Segment;
					if (Segment && MmIsAddressValid(Segment))
					{
						ControlArea = Segment->ControlArea;
						if (ControlArea && MmIsAddressValid(ControlArea))
						{
							FileObject = ControlArea->FilePointer;

							if (FileObject&&MmIsAddressValid(FileObject))
							{
								bGetPath = GetPathByFileObject(FileObject, wzProcessPath);
								if (!bGetPath)
								{
									DbgPrint("SectionObject: 0x%08X, FileObject: 0x%08X\n", SectionObject, FileObject);
								}
							}
						}
					}
				}
			}
			break;
		}

		case WINDOWS_7:
		{
			SectionObjectOffsetOfEProcess = 0x268;


			if (SectionObjectOffsetOfEProcess != 0 && MmIsAddressValid((PVOID)((ULONG_PTR)EProcess + SectionObjectOffsetOfEProcess)))
			{
				SectionObject64 = *(PSECTION_OBJECT64*)((ULONG_PTR)EProcess + SectionObjectOffsetOfEProcess);



				if (SectionObject64 && MmIsAddressValid(SectionObject64))
				{

					Segment64 = (PSEGMENT64)(SectionObject64->Segment);
					if (Segment64 && MmIsAddressValid(Segment64))
					{
						ControlArea64 = (PCONTROL_AREA64)Segment64->ControlArea;
						if (ControlArea64 && MmIsAddressValid(ControlArea64))
						{
							FileObject = (PFILE_OBJECT)ControlArea64->FilePointer;

							if (FileObject&&MmIsAddressValid(FileObject))
							{
								FileObject = (PFILE_OBJECT)((ULONG_PTR)FileObject & 0xFFFFFFFFFFFFFFF0);
								bGetPath = GetPathByFileObject(FileObject, wzProcessPath);
								if (!bGetPath)
								{
									DbgPrint("SectionObject: 0x%08X, FileObject: 0x%08X\n", SectionObject, FileObject);
								}
							}
						}
					}
				}
			}
			break;
		}
		}
	}

	if (bGetPath == FALSE)
	{
		wcscpy(wzProcessPath, L"Unknow");
	}

	return bGetPath;
}

BOOLEAN GetPathByFileObject(PFILE_OBJECT FileObject, WCHAR* wzPath)
{
	BOOLEAN bGetPath = FALSE;
	CHAR szIoQueryFileDosDeviceName[] = "IoQueryFileDosDeviceName";
	CHAR szIoVolumeDeviceToDosName[] = "IoVolumeDeviceToDosName";
	CHAR szRtlVolumeDeviceToDosName[] = "RtlVolumeDeviceToDosName";

	POBJECT_NAME_INFORMATION ObjectNameInformation = NULL;
	__try
	{
		if (FileObject && MmIsAddressValid(FileObject) && wzPath)
		{

			if (NT_SUCCESS(IoQueryFileDosDeviceName(FileObject, &ObjectNameInformation)))   //ע��ú������ú�Ҫ�ͷ��ڴ�
			{
				wcsncpy(wzPath, ObjectNameInformation->Name.Buffer, ObjectNameInformation->Name.Length);

				bGetPath = TRUE;

				ExFreePool(ObjectNameInformation);
			}

			if (!bGetPath)
			{

				if (IoVolumeDeviceToDosName || RtlVolumeDeviceToDosName)
				{
					NTSTATUS  Status = STATUS_UNSUCCESSFUL;
					ULONG_PTR ulRet = 0;
					PVOID     Buffer = ExAllocatePool(PagedPool, 0x1000);

					if (Buffer)
					{
						// ObQueryNameString : \Device\HarddiskVolume1\Program Files\VMware\VMware Tools\VMwareTray.exe
						memset(Buffer, 0, 0x1000);
						Status = ObQueryNameString(FileObject, (POBJECT_NAME_INFORMATION)Buffer, 0x1000, &ulRet);
						if (NT_SUCCESS(Status))
						{
							POBJECT_NAME_INFORMATION Temp = (POBJECT_NAME_INFORMATION)Buffer;

							WCHAR szHarddiskVolume[100] = L"\\Device\\HarddiskVolume";

							if (Temp->Name.Buffer != NULL)
							{
								if (Temp->Name.Length / sizeof(WCHAR) > wcslen(szHarddiskVolume) &&
									!_wcsnicmp(Temp->Name.Buffer, szHarddiskVolume, wcslen(szHarddiskVolume)))
								{
									// ������� "\\Device\\HarddiskVolume" ��������ʽ���ڵģ���ô�ٲ�ѯ�������
									UNICODE_STRING uniDosName;

									if (NT_SUCCESS(IoVolumeDeviceToDosName(FileObject->DeviceObject, &uniDosName)))
									{
										if (uniDosName.Buffer != NULL)
										{

											wcsncpy(wzPath, uniDosName.Buffer, uniDosName.Length);
											wcsncat(wzPath, Temp->Name.Buffer + wcslen(szHarddiskVolume) + 1, Temp->Name.Length - (wcslen(szHarddiskVolume) + 1));
											bGetPath = TRUE;
										}

										ExFreePool(uniDosName.Buffer);
									}

									else if (NT_SUCCESS(RtlVolumeDeviceToDosName(FileObject->DeviceObject, &uniDosName)))
									{
										if (uniDosName.Buffer != NULL)
										{

											wcsncpy(wzPath, uniDosName.Buffer, uniDosName.Length);
											wcsncat(wzPath, Temp->Name.Buffer + wcslen(szHarddiskVolume) + 1, Temp->Name.Length - (wcslen(szHarddiskVolume) + 1));
											bGetPath = TRUE;
										}

										ExFreePool(uniDosName.Buffer);
									}

								}
								else
								{
									// ��������� "\\Device\\HarddiskVolume" ��������ʽ��ͷ�ģ���ôֱ�Ӹ������ơ�

									wcsncpy(wzPath, Temp->Name.Buffer, Temp->Name.Length);
									bGetPath = TRUE;
								}
							}
						}

						ExFreePool(Buffer);
					}
				}
			}
		}
	}
	__except (1)
	{
		DbgPrint("HsGetPathByFileObject Catch __Except\r\n");
		bGetPath = FALSE;
	}

	return bGetPath;
}

PEPROCESS GetIdleEProcess()
{
	ULONG_PTR uIdleAddr = 0;
	ULONG_PTR PsInitialSystemProcessAddress = (ULONG_PTR)&PsInitialSystemProcess;


	DbgPrint("%x\r\n", PsInitialSystemProcessAddress);
	switch (WinVersion)
	{
	case WINDOWS_7:
	{
		if (PsInitialSystemProcessAddress && MmIsAddressValid((PVOID)((ULONG_PTR)PsInitialSystemProcessAddress + 0xA0)))
		{
			uIdleAddr = *(PULONG_PTR)((ULONG_PTR)PsInitialSystemProcessAddress + 0xA0);	//0xA0ԭ����

			if (uIdleAddr <= 0xffff)
			{
				uIdleAddr = *(PULONG_PTR)((ULONG_PTR)PsInitialSystemProcessAddress + 0xB0);	//0xB0���º��
			}
		}
		break;
	}
	case WINDOWS_XP:
	{
		if (PsInitialSystemProcessAddress && MmIsAddressValid((PVOID)((ULONG_PTR)PsInitialSystemProcessAddress - 0x78B4)))
		{
			uIdleAddr = (ULONG_PTR)((ULONG_PTR)PsInitialSystemProcessAddress - 0x78B4);
		}
		break;
	}
	}

	DbgPrint("IdleEProcess:%p\r\n", uIdleAddr);

	return (PEPROCESS)uIdleAddr;

}

ULONG_PTR GetParentProcessIdByEProcess(PEPROCESS EProcess)
{
	if (MmIsAddressValid &&
		EProcess &&
		MmIsAddressValid(EProcess) &&
		MmIsAddressValid((PVOID)((ULONG_PTR)EProcess + ObjectTableOffsetOf_EPROCESS)))
	{
		ULONG_PTR ulParentPid = 0;

		ulParentPid = *(ULONG_PTR*)((ULONG_PTR)EProcess + ParentProcessIdOffset);

		return ulParentPid;
	}

	return 0;
}

//�ڴ����㷨��������
NTSTATUS KillProcessByZeroMemory(ULONG_PTR ProcessID, int* bFeedBack)
{

	OBJECT_ATTRIBUTES oa = { 0 };
	CLIENT_ID         Cid = { 0 };
	NTSTATUS  Status;
	HANDLE hProcess = NULL;

	Cid.UniqueProcess = (HANDLE)ProcessID;
	Cid.UniqueThread = 0;

	Status = ZwOpenProcess(&hProcess, GENERIC_ALL, &oa, &Cid);

	if (!NT_SUCCESS(Status))
	{

		return FALSE;
	}

	ZwTerminateProcess(hProcess, 0);   //Sys

	ZwClose(hProcess);

	*bFeedBack = TRUE;

	return Status;
}

CHAR ChangePreMode(PETHREAD EThread)
{

	CHAR PreMode = *(PCHAR)((ULONG_PTR)EThread + PreviousModeOffsetOf_KTHREAD);
	*(PCHAR)((ULONG_PTR)EThread + PreviousModeOffsetOf_KTHREAD) = KernelMode;
	return PreMode;
}


VOID RecoverPreMode(PETHREAD EThread, CHAR PreMode)
{
	*(PCHAR)((ULONG_PTR)EThread + PreviousModeOffsetOf_KTHREAD) = PreMode;
}
