#pragma once

class CusbHIDDlg;  //声明主对话框类
class CUsbHIDRead
{
public:
	CUsbHIDRead(void);
	~CUsbHIDRead(void);

	//线程相关
public:
	HANDLE m_HWorkerThread;  //工作者线程的句柄指针
	void *m_pThreadParam;  //线程参数，可以根据需要定义自己的结构体传入线程，一般是包含对象this的结构
	bool m_bThreadExit;  //线程退出标识
	DWORD m_dwThreadID;  //线程ID
	CString m_strThreadName;  //线程名称，调试用
	
public:
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);  //线程函数
	bool Start(HANDLE hUsbHid, CusbHIDDlg *pMainDlg);
	//bool Puase();
	void Exit();
	//线程相关-END

	//要操作设备或数据
	HANDLE m_hUsbHid;

	//界面操作入口
	CusbHIDDlg *m_pMainDlg;
};

