
// PCAssistant.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPCAssistantApp: 
// �йش����ʵ�֣������ PCAssistant.cpp
//

class CPCAssistantApp : public CWinApp
{
public:
	CPCAssistantApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPCAssistantApp theApp;