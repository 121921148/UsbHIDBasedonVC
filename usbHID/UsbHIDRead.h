#pragma once

class CusbHIDDlg;  //�������Ի�����
class CUsbHIDRead
{
public:
	CUsbHIDRead(void);
	~CUsbHIDRead(void);

	//�߳����
public:
	HANDLE m_HWorkerThread;  //�������̵߳ľ��ָ��
	void *m_pThreadParam;  //�̲߳��������Ը�����Ҫ�����Լ��Ľṹ�崫���̣߳�һ���ǰ�������this�Ľṹ
	bool m_bThreadExit;  //�߳��˳���ʶ
	DWORD m_dwThreadID;  //�߳�ID
	CString m_strThreadName;  //�߳����ƣ�������
	
public:
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);  //�̺߳���
	bool Start(HANDLE hUsbHid, CusbHIDDlg *pMainDlg);
	//bool Puase();
	void Exit();
	//�߳����-END

	//Ҫ�����豸������
	HANDLE m_hUsbHid;

	//����������
	CusbHIDDlg *m_pMainDlg;
};

