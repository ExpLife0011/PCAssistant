#pragma once
#include "stdafx.h"


typedef enum _FILTER_TYPE_
{
	Unkonw,
	File,
	Disk,
	Volume,
	Keyboard,
	Mouse,
	I8042prt,
	Tcpip,
	NDIS,
	PnpManager,
	Tdx,
	RAW
}FILTER_TYPE;

typedef struct _FILTER_INFO_
{
	FILTER_TYPE Type;
	ULONG_PTR FileterDeviceObject;
	WCHAR wzFilterDriverName[100];
	WCHAR wzAttachedDriverName[100];
	WCHAR wzPath[MAX_PATH];
}FILTER_INFO, *PFILTER_INFO;

typedef struct _FILTER_DRIVER_
{
	ULONG_PTR ulCnt;
	ULONG_PTR ulRetCnt;
	FILTER_INFO Filter[1];
}FILTER_DRIVER, *PFILTER_DRIVER;

typedef struct _UNLOAD_FILTER_
{
	FILTER_TYPE Type;
	ULONG_PTR   DeviceObject;
}UNLOAD_FILTER, *PUNLOAD_FILTER;

VOID InitFilterDriverList(CListCtrl *m_ListCtrl);
VOID LoadFilterDriverList(CListCtrl *m_ListCtrl);
VOID QueryFilterDriverList(CListCtrl *m_ListCtrl);
void InsertFilterDriverItem(CListCtrl* m_ListCtrl);
VOID RemoveFilterDriverItem(CListCtrl* m_ListCtrl);

