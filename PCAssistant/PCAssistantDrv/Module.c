#include "Module.h"
#include "GetFuncAddress.h"
#include "Process.h"

extern     PDRIVER_OBJECT g_DriverObject;
extern     PDEVICE_OBJECT g_DeviceObject;

extern
WIN_VERSION  WinVersion;
extern
ULONG_PTR ulBuildNumber;
extern
ULONG_PTR	PreviousModeOffsetOf_KTHREAD;
extern
ULONG_PTR	ObjectTableOffsetOf_EPROCESS;
extern
ULONG_PTR	SYSTEM_ADDRESS_START;
extern
ULONG_PTR	ObjectHeaderSize;
extern
ULONG_PTR	ObjectTypeOffsetOf_Object_Header;

PVOID
Ntoskrnl_KLDR_DATA_TABLE_ENTRY = NULL;

pfnNtOpenDirectoryObject  NtOpenDirectoryObjectAddress = NULL;

POBJECT_TYPE DirectoryObjectType = NULL;

extern POBJECT_TYPE *IoDriverObjectType;
extern POBJECT_TYPE *IoDeviceObjectType;

NTSTATUS DispatchControlForModule(PIO_STACK_LOCATION IrpSp, PVOID OutputBuffer, ULONG_PTR * ulRet)
{
	WCHAR* szOutputBuffer = (WCHAR*)OutputBuffer;
	ULONG				ulIoControlCode = 0;
	NTSTATUS			Status = STATUS_UNSUCCESSFUL;
	PVOID               pvInputBuffer = NULL;
	ULONG               ulInputLen = 0;
	ULONG				ulOutputLen = 0;

	pvInputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
	ulInputLen = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	ProbeForRead(pvInputBuffer, ulInputLen, sizeof(CHAR));

	ulOutputLen = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

	ProbeForWrite(OutputBuffer, ulOutputLen, sizeof(CHAR));

	ulIoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
	ulIoControlCode = (ulIoControlCode >> 2) & 0x00000FFF;

	DbgPrint("%x\r\n", ulIoControlCode);

	InitModuleGlobalVariable();

	switch (ulIoControlCode)
	{
	
	case IOCTL_MODU_MODULELIST:
	{
		DbgPrint("IOCTL_MODU_MODULELIST\r\n");

		NtOpenDirectoryObjectAddress = (pfnNtOpenDirectoryObject)GetFuncAddress("NtOpenDirectoryObject");
		GetKernelLdrDataTableEntry(g_DriverObject);
		Status = EnumSystemModuleList(OutputBuffer, ulOutputLen);
		break;
	}
	case IOCTL_MODU_REMOVEMODULE:
	{
		DbgPrint("IOCTL_MODU_REMOVEMODULE\r\n");

		if (pvInputBuffer != NULL&&ulInputLen == sizeof(ULONG_PTR))
		{
			Status = UnloadDriverModule(*(PULONG_PTR)pvInputBuffer, ulInputLen);
		}
		break;
	}

	default:
	{
		Status = STATUS_UNSUCCESSFUL;
		break;
	}
		
	}
	return Status;
}

BOOLEAN GetKernelLdrDataTableEntry(PDRIVER_OBJECT DriverObject)
{
	BOOLEAN bRet = FALSE;
	if (DriverObject)
	{
		PKLDR_DATA_TABLE_ENTRY Entry = NULL, FirstEntry = NULL;
		WCHAR wzNtoskrnl[] = L"ntoskrnl.exe";
		int nLen = wcslen(wzNtoskrnl) * sizeof(WCHAR);

		FirstEntry = Entry = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;

		while ((PKLDR_DATA_TABLE_ENTRY)Entry->InLoadOrderLinks.Flink != FirstEntry)
		{

			if (Entry->BaseDllName.Buffer								&&
				nLen == Entry->BaseDllName.Length						&&
				MmIsAddressValid((PVOID)Entry->BaseDllName.Buffer) &&
				!_wcsnicmp(wzNtoskrnl, (WCHAR*)Entry->BaseDllName.Buffer, nLen / sizeof(WCHAR)))
			{
				Ntoskrnl_KLDR_DATA_TABLE_ENTRY = (PVOID)Entry;
				bRet = TRUE;
				break;
			}

			Entry = (PKLDR_DATA_TABLE_ENTRY)Entry->InLoadOrderLinks.Flink;
		}

		// ���ʵ��û�ҵ�ntoskrnl,��ôʹ���Լ���
		if (!bRet)
		{
			Ntoskrnl_KLDR_DATA_TABLE_ENTRY = (PVOID)FirstEntry;
			bRet = TRUE;
		}
	}

	return bRet;
}


NTSTATUS EnumSystemModuleList(PVOID OutBuffer, ULONG OutSize)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PALL_DRIVERS DriversInfor = (PALL_DRIVERS)OutBuffer;
	ULONG ulCount = (OutSize - sizeof(ALL_DRIVERS)) / sizeof(DRIVER_INFO);

	// ������
	if (!OutBuffer)
	{
		return STATUS_INVALID_PARAMETER;
	}

	EnumDriverByLdrDataTableEntry(DriversInfor, ulCount);
	EnumDriversByWalkerDirectoryObject(DriversInfor, ulCount);
	if (ulCount >= DriversInfor->ulCount)
	{
		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = STATUS_BUFFER_TOO_SMALL;
	}

	return Status;
}

VOID EnumDriverByLdrDataTableEntry(PALL_DRIVERS DriversInfor, ULONG_PTR ulCount)
{
	PKLDR_DATA_TABLE_ENTRY Entry = NULL, FirstEntry = NULL;
	ULONG nMax = PAGE_SIZE;
	ULONG i = 0;
	KIRQL OldIrql;

	FirstEntry = Entry = (PKLDR_DATA_TABLE_ENTRY)Ntoskrnl_KLDR_DATA_TABLE_ENTRY;

	if (!FirstEntry || !DriversInfor)
	{
		return;
	}

	OldIrql = KeRaiseIrqlToDpcLevel();

	__try
	{
		do
		{
			if ((ULONG_PTR)Entry->DllBase > SYSTEM_ADDRESS_START && Entry->SizeOfImage > 0)
			{
				ULONG_PTR Temp = DriversInfor->ulCount;
				if (ulCount > Temp)
				{

					DriversInfor->Drivers[Temp].LodeOrder = ++i;
					DriversInfor->Drivers[Temp].Base = (ULONG_PTR)Entry->DllBase;
					DriversInfor->Drivers[Temp].Size = Entry->SizeOfImage;

					if (IsUnicodeStringValid(&(Entry->FullDllName)))
					{
						memcpy(DriversInfor->Drivers[Temp].wzDriverPath, (WCHAR*)Entry->FullDllName.Buffer, Entry->FullDllName.Length);
					}
					else if (IsUnicodeStringValid(&(Entry->BaseDllName)))
					{
						memcpy(DriversInfor->Drivers[Temp].wzDriverPath, (WCHAR*)Entry->BaseDllName.Buffer, Entry->BaseDllName.Length);
					}

				}

				DriversInfor->ulCount++;
			}

			Entry = (PKLDR_DATA_TABLE_ENTRY)Entry->InLoadOrderLinks.Flink;

		} while (Entry && Entry != FirstEntry && nMax--);
	}
	__except (1)
	{}

	KeLowerIrql(OldIrql);
}


VOID EnumDriversByWalkerDirectoryObject(PALL_DRIVERS DriversInfor, ULONG_PTR ulCount)
{
	NTSTATUS Status;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING uniDirectory;
	HANDLE hDirectory;
	PVOID  DirectoryObject = NULL;
	WCHAR  wzDirectory[] = { L'\\', L'\0' };
	PETHREAD EThread = NULL;
	CHAR PreMode = 0;


	RtlInitUnicodeString(&uniDirectory, wzDirectory);
	InitializeObjectAttributes(&oa, &uniDirectory, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	EThread = PsGetCurrentThread();
	PreMode = ChangePreMode(EThread);

	Status = NtOpenDirectoryObjectAddress(&hDirectory, 0, &oa);
	if (NT_SUCCESS(Status))
	{
		Status = ObReferenceObjectByHandle(hDirectory, 0x10000000, 0, 0, &DirectoryObject, 0);
		if (NT_SUCCESS(Status))
		{

			DirectoryObjectType = KeGetObjectTypeInModule(DirectoryObject);

			WalkerDirectoryObject(DriversInfor, DirectoryObject, ulCount);
			ObfDereferenceObject(DirectoryObject);
		}

		Status = NtClose(hDirectory);
	}

	RecoverPreMode(EThread, PreMode);
}

VOID WalkerDirectoryObject(PALL_DRIVERS DriversInfor, PVOID DirectoryObject, ULONG_PTR ulCount)
{
	if (DirectoryObject && 
		DriversInfor	&& 
		MmIsAddressValid(DirectoryObject))
	{
		ULONG i = 0;
		POBJECT_DIRECTORY ObjectDir = (POBJECT_DIRECTORY)DirectoryObject;
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();

		__try
		{
			for (i = 0; i < NUMBER_HASH_BUCKETS; i++)
			{
				POBJECT_DIRECTORY_ENTRY ObjectDirEntry = ObjectDir->HashBuckets[i];
				for (; (ULONG_PTR)ObjectDirEntry > SYSTEM_ADDRESS_START && MmIsAddressValid(ObjectDirEntry); ObjectDirEntry = ObjectDirEntry->ChainLink)
				{
					if (MmIsAddressValid(ObjectDirEntry->Object))
					{
						POBJECT_TYPE ObjectType = KeGetObjectTypeInModule(ObjectDirEntry->Object);

						// �����Ŀ¼����ô�����ݹ����
						if (ObjectType == DirectoryObjectType)
						{
							WalkerDirectoryObject(DriversInfor, ObjectDirEntry->Object, ulCount);
						}

						// �������������
						else if (ObjectType == *IoDriverObjectType)
						{
							PDEVICE_OBJECT DeviceObject = NULL;

							if (!IsDriverInList(DriversInfor, (PDRIVER_OBJECT)ObjectDirEntry->Object, ulCount))
							{
								InsertDriver(DriversInfor, (PDRIVER_OBJECT)ObjectDirEntry->Object, ulCount);
							}

							// �����豸ջ
							for (DeviceObject = ((PDRIVER_OBJECT)ObjectDirEntry->Object)->DeviceObject;
								DeviceObject && MmIsAddressValid(DeviceObject);
								DeviceObject = DeviceObject->AttachedDevice)
							{
								if (!IsDriverInList(DriversInfor, DeviceObject->DriverObject, ulCount))
								{
									InsertDriver(DriversInfor, DeviceObject->DriverObject, ulCount);
								}
							}
						}

						// ������豸����
						else if (ObjectType == *IoDeviceObjectType)
						{
							PDEVICE_OBJECT DeviceObject = NULL;

							if (!IsDriverInList(DriversInfor, ((PDEVICE_OBJECT)ObjectDirEntry->Object)->DriverObject, ulCount))
							{
								InsertDriver(DriversInfor, ((PDEVICE_OBJECT)ObjectDirEntry->Object)->DriverObject, ulCount);
							}

							// �����豸ջ
							for (DeviceObject = ((PDEVICE_OBJECT)ObjectDirEntry->Object)->AttachedDevice;
								DeviceObject && MmIsAddressValid(DeviceObject);
								DeviceObject = DeviceObject->AttachedDevice)
							{
								if (!IsDriverInList(DriversInfor, DeviceObject->DriverObject, ulCount))
								{
									InsertDriver(DriversInfor, DeviceObject->DriverObject, ulCount);
								}
							}
						}
					}
				}
			}
		}
		__except (1)
		{
		}

		KeLowerIrql(OldIrql);
	}
}

BOOLEAN IsDriverInList(PALL_DRIVERS DriversInfor, PDRIVER_OBJECT DriverObject, ULONG_PTR ulCount)
{
	BOOLEAN bIn = TRUE, bFind = FALSE;

	if (!DriversInfor ||
		!DriverObject ||
		!MmIsAddressValid(DriverObject))
	{
		return TRUE;
	}

	__try
	{
		if (MmIsAddressValid(DriverObject))
		{
			PKLDR_DATA_TABLE_ENTRY Entry = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;

			if (Entry &&
				MmIsAddressValid(Entry) &&
				MmIsAddressValid((PVOID)Entry->DllBase) &&
				(ULONG_PTR)Entry->DllBase > SYSTEM_ADDRESS_START)
			{
				ULONG i = 0;
				ULONG Temp = ulCount > DriversInfor->ulCount ? DriversInfor->ulCount : ulCount;

				for (i = 0; i < Temp; i++)
				{
					if (DriversInfor->Drivers[i].Base == (ULONG_PTR)Entry->DllBase)
					{
						if (DriversInfor->Drivers[i].DriverObject == 0)
						{
							//�����������
							DriversInfor->Drivers[i].DriverObject = (ULONG_PTR)DriverObject;

							//����������
							DriversInfor->Drivers[i].DirverStartAddress = (ULONG_PTR)Entry->EntryPoint;

							//��÷�����
							wcsncpy(DriversInfor->Drivers[i].wzKeyName, DriverObject->DriverExtension->ServiceKeyName.Buffer, DriverObject->DriverExtension->ServiceKeyName.Length);
						}

						bFind = TRUE;
						break;
					}
				}

				if (!bFind)
				{
					bIn = FALSE;
				}
			}
		}
	}
	__except (1)
	{
		bIn = TRUE;
	}

	return bIn;
}

VOID InsertDriver(PALL_DRIVERS DriversInfor, PDRIVER_OBJECT DriverObject, ULONG_PTR ulCount)
{
	if (!DriversInfor || !DriverObject || !MmIsAddressValid(DriverObject))
	{
		return;
	}
	else
	{
		PKLDR_DATA_TABLE_ENTRY Entry = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;

		if (Entry &&
			MmIsAddressValid(Entry) &&
			MmIsAddressValid((PVOID)Entry->DllBase) &&
			(ULONG_PTR)Entry->DllBase > SYSTEM_ADDRESS_START)
		{
			ULONG Temp = DriversInfor->ulCount;
			if (ulCount > Temp)
			{
				DriversInfor->Drivers[Temp].Base = (ULONG_PTR)Entry->DllBase;
				DriversInfor->Drivers[Temp].Size = Entry->SizeOfImage;
				DriversInfor->Drivers[Temp].DriverObject = (ULONG_PTR)DriverObject;

				if (IsUnicodeStringValid(&(Entry->FullDllName)))
				{
					wcsncpy(DriversInfor->Drivers[Temp].wzDriverPath, (WCHAR*)(Entry->FullDllName.Buffer), Entry->FullDllName.Length);
				}
				else if (IsUnicodeStringValid(&(Entry->BaseDllName)))
				{
					wcsncpy(DriversInfor->Drivers[Temp].wzDriverPath, (WCHAR*)(Entry->BaseDllName.Buffer), Entry->BaseDllName.Length);
				}
			}
			DriversInfor->ulCount++;
		}
	}
}



POBJECT_TYPE KeGetObjectTypeInModule(PVOID Object)
{
	ULONG_PTR				ObjectType = NULL;
	pfnObGetObjectType		ObGetObjectType = NULL;

	if (!Object || !MmIsAddressValid(Object))
	{
		return NULL;
	}

	if (ulBuildNumber < 6000)
	{
		ULONG ObjectTypeAddress = 0;

		ObjectTypeAddress = (ULONG_PTR)Object - ObjectHeaderSize + ObjectTypeOffsetOf_Object_Header;

		if (MmIsAddressValid((PVOID)ObjectTypeAddress))
		{
			ObjectType = *(ULONG_PTR*)ObjectTypeAddress;
		}
	}
	else
	{
		ObGetObjectType = (pfnObGetObjectType)GetFunctionAddressByName(L"ObGetObjectType");


		if (ObGetObjectType)
		{
			ObjectType = ObGetObjectType(Object);
		}
	}

	return (POBJECT_TYPE)ObjectType;
}

VOID InitModuleGlobalVariable()
{
	switch (WinVersion)
	{
	case WINDOWS_XP:
	{
		PreviousModeOffsetOf_KTHREAD = 0x140;
		ObjectHeaderSize = 0x18;
		ObjectTypeOffsetOf_Object_Header = 0x8;
		ObjectTableOffsetOf_EPROCESS = 0x0c4;
		SYSTEM_ADDRESS_START = 0x80000000;
		break;
	}

	case WINDOWS_7:
	{
		PreviousModeOffsetOf_KTHREAD = 0x1f6;
		ObjectTableOffsetOf_EPROCESS = 0x200;
		ObjectHeaderSize = 0x30;
		SYSTEM_ADDRESS_START = 0x80000000000;
		break;
	}
	}
}


NTSTATUS UnloadDriverModule(ULONG_PTR InBuffer, ULONG_PTR InSize)
{

	PDRIVER_OBJECT DriverObject = (PDRIVER_OBJECT)InBuffer;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	DbgPrint("g_DriverObject: %p\r\n", g_DriverObject);
	DbgPrint("  DriverObject: %p\r\n", DriverObject);

	if ((ULONG_PTR)DriverObject > SYSTEM_ADDRESS_START &&
		MmIsAddressValid(DriverObject) &&
		g_DriverObject != DriverObject &&
		IsRealDriverObject(DriverObject))
	{
		Status = PspUnloadDriver(DriverObject);
	}

	return Status;
}

//�ж�һ�������Ƿ�Ϊ�����������
BOOLEAN IsRealDriverObject(PDRIVER_OBJECT DriverObject)
{
	BOOLEAN bRet = FALSE;
	if (!*IoDriverObjectType ||
		!*IoDeviceObjectType)
	{
		return bRet;
	}

	__try
	{
		if (DriverObject->Type == 4 &&
			DriverObject->Size == sizeof(DRIVER_OBJECT) &&
			KeGetObjectTypeInModule(DriverObject) == *IoDriverObjectType &&
			MmIsAddressValid(DriverObject->DriverSection) &&
			(ULONG_PTR)DriverObject->DriverSection > SYSTEM_ADDRESS_START &&
			!(DriverObject->DriverSize & 0x1F) &&
			DriverObject->DriverSize < SYSTEM_ADDRESS_START &&
			!((ULONG_PTR)(DriverObject->DriverStart) & 0xFFF) &&
			(ULONG_PTR)DriverObject->DriverStart > SYSTEM_ADDRESS_START
			)
		{
			PDEVICE_OBJECT DeviceObject = DriverObject->DeviceObject;
			if (DeviceObject)
			{
				if (MmIsAddressValid(DeviceObject) &&
					KeGetObjectTypeInModule(DeviceObject) == *IoDeviceObjectType &&
					DeviceObject->Type == 3 &&
					DeviceObject->Size >= sizeof(DEVICE_OBJECT))
				{
					bRet = TRUE;
				}
			}
			else
			{
				bRet = TRUE;
			}
		}
	}
	__except (1)
	{
		bRet = FALSE;
	}

	return bRet;
}

NTSTATUS PspUnloadDriver(PDRIVER_OBJECT DriverObject)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;


	if (MmIsAddressValid(DriverObject))
	{
		BOOLEAN bDriverUnload = FALSE;
		HANDLE  hSystemThread = NULL;

		if (DriverObject->DriverUnload &&
			(ULONG_PTR)DriverObject->DriverUnload > SYSTEM_ADDRESS_START &&
			MmIsAddressValid(DriverObject->DriverUnload))
		{
			bDriverUnload = TRUE;
		}

		if (bDriverUnload)
		{
			Status = PsCreateSystemThread(&hSystemThread, 0, NULL, NULL, NULL, HaveDriverUnloadThread, DriverObject);  //�������ж�غ���
																													   //ж������������û������
																													   //ж��32λXutre���������� �������ǵ�ԭ�� Xutre��UnloadDriverû�д����
		}
		else
		{
			Status = PsCreateSystemThread(&hSystemThread, 0, NULL, NULL, NULL, NotHaveDriverUnloadThread, DriverObject);
		}

		if (NT_SUCCESS(Status))
		{
			PETHREAD EThread = NULL, CurrentThread = NULL;
			CHAR PreMode = 0;

			Status = ObReferenceObjectByHandle(hSystemThread, 0, NULL, KernelMode, &EThread, NULL);
			if (NT_SUCCESS(Status))
			{
				LARGE_INTEGER TimeOut;
				TimeOut.QuadPart = -10 * 1000 * 1000 * 3;
				Status = KeWaitForSingleObject(EThread, Executive, KernelMode, TRUE, &TimeOut); // �ȴ�3��
				ObfDereferenceObject(EThread);
			}

			CurrentThread = PsGetCurrentThread();
			PreMode = ChangePreMode(CurrentThread);
			NtClose(hSystemThread);
			RecoverPreMode(CurrentThread, PreMode);
		}
	}

	return Status;
}

VOID HaveDriverUnloadThread(PVOID lParam)
{

	PDRIVER_OBJECT DriverObject = (PDRIVER_OBJECT)lParam;

	if (DriverObject)
	{
		PDRIVER_UNLOAD DriverUnloadAddress = DriverObject->DriverUnload;

		if (DriverUnloadAddress)
		{
			DriverUnloadAddress(DriverObject);



			DriverObject->FastIoDispatch = NULL;
			memset(DriverObject->MajorFunction, 0, sizeof(DriverObject->MajorFunction));
			DriverObject->DriverUnload = NULL;

			ObMakeTemporaryObject(DriverObject);
			ObfDereferenceObject(DriverObject);
		}
	}

	PsTerminateSystemThread(STATUS_SUCCESS);
}


VOID NotHaveDriverUnloadThread(IN PVOID lParam)
{

	PDRIVER_OBJECT DriverObject = (PDRIVER_OBJECT)lParam;
	PDEVICE_OBJECT DeviceObject = NULL;

	if (DriverObject)
	{

		DriverObject->FastIoDispatch = NULL;
		memset(DriverObject->MajorFunction, 0, sizeof(DriverObject->MajorFunction));
		DriverObject->DriverUnload = NULL;

		DeviceObject = DriverObject->DeviceObject;

		while (DeviceObject && MmIsAddressValid(DeviceObject))
		{
			IoDeleteDevice(DeviceObject);
			DeviceObject = DeviceObject->NextDevice;
		}

		ObMakeTemporaryObject(DriverObject);
		ObfDereferenceObject(DriverObject);
	}

	PsTerminateSystemThread(STATUS_SUCCESS);
}