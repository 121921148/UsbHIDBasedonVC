
// usbHID.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CusbHIDApp:
// �йش����ʵ�֣������ usbHID.cpp
//

class CusbHIDApp : public CWinApp
{
public:
	CusbHIDApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CusbHIDApp theApp;