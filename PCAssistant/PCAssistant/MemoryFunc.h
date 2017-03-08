#pragma once
#include "stdafx.h"

typedef struct _PROTECT_
{
	ULONG uType;
	WCHAR szTypeName[50];
}PROTECT, *PPROTECT;


typedef struct _MEMORY_INFO_
{
	ULONG_PTR ulBase;
	ULONG_PTR ulSize;
	ULONG ulProtect;
	ULONG ulState;
	ULONG ulType;
}MEMORY_INFO, *PMEMORY_INFO;

typedef struct _ALL_MEMORYS_
{
	ULONG_PTR ulCount;
	MEMORY_INFO Memorys[1];
}ALL_MEMORYS, *PALL_MEMORYS;

typedef enum _MEMORY_HEADER_INDEX
{
	MemoryBase,
	MemorySize,
	MemoryProtect,
	MemoryState,
	MemoryType,
	MmeoryModuleName,
}MEMORY_HEADER_INDEX;

void InitMemoryList(CListCtrl *m_ListCtrl);
BOOL QueryProcessMemory(CListCtrl *m_ListCtrl);
BOOL SendIoControlCodeModule(ULONG_PTR ProcessID);
BOOL SendIoControlCodeMemory(ULONG_PTR ProcessID);
VOID InitMemoryProtect();
CString GetMemoryProtect(ULONG Protect);
CString GetMemoryType(ULONG Type);
CString GetMemoryState(ULONG State);
CString GetModuleImageName(ULONG_PTR ulBase);
