#include "StdAfx.h"
#include "UsbHIDRead.h"
#include "usbHID.h"
#include "usbHIDDlg.h"


CUsbHIDRead::CUsbHIDRead(void)
{
	m_HWorkerThread = INVALID_HANDLE_VALUE;
	m_pThreadParam = this;
	m_bThreadExit = false;
	m_strThreadName = "CGPSTcpserver";

	m_hUsbHid = INVALID_HANDLE_VALUE;
}


CUsbHIDRead::~CUsbHIDRead(void)
{
}

DWORD WINAPI CUsbHIDRead::_WorkerThread(LPVOID lpParam)
{
	if (lpParam == NULL)
	{
		TRACE("CUsbHIDRead::_WorkerThread() 获取线程参数失败\n");
	}
	CUsbHIDRead *pDS = (CUsbHIDRead*)lpParam;

	CString strRecv, strTemp;

	TRACE("%s(%d) start\n", pDS->m_strThreadName, pDS->m_dwThreadID);

	bool bAsynRW = pDS->m_pMainDlg->m_usbHID.m_bAsynRW;
	OVERLAPPED *pOverLapped = NULL;
	if (bAsynRW)
	{
		pOverLapped = &pDS->m_pMainDlg->m_usbHID.m_USBReadOverlapped;
	}

	while(true)
	{
		if (pDS->m_bThreadExit)
		{
			//线程退出清理工作
			pDS->m_bThreadExit = false;
			CloseHandle(pDS->m_HWorkerThread);
			pDS->m_HWorkerThread = INVALID_HANDLE_VALUE;

			TRACE("%s(%d) exit\n", pDS->m_strThreadName, pDS->m_dwThreadID);

			break;  //退出线程函数，终止线程
		}

		if (pDS->m_hUsbHid != INVALID_HANDLE_VALUE)
		{
			unsigned char byRecvBuf[20];
			BOOL bRet = FALSE;
			unsigned long nBytesRead=0;

			if (bAsynRW)  //异步读写
			{
				ResetEvent(pOverLapped->hEvent);
				bRet = ReadFile(pDS->m_hUsbHid, byRecvBuf, 8, &nBytesRead, pOverLapped);

				if (!bRet)
				{
					if (ERROR_IO_PENDING == GetLastError()) //函数返回ERROR_IO_PENDING,表明USB正在进行读操作
					{
						TRACE("等待HID设备读操作....\n");
						//使用WaitForSingleObject函数等待，直到读操作完成或延时已达到2秒钟.当USB读操作进行完毕后，USBReadOverlapped的hEvent事件会变为有信号
						if (WAIT_OBJECT_0 == WaitForSingleObject(pOverLapped->hEvent, 2000))
						{
							for (int i = 0; i < 8; i++)
							{
								strTemp.Format("%.2X \n", byRecvBuf[i]);
								strRecv = strRecv + strTemp;
							}

							pDS->m_pMainDlg->m_editRecv.ReplaceSel(strRecv);
							TRACE("完成HID设备读操作!!!\n");
						}
						else
						{
							TRACE("*****HID设备读操作------------超时\n");
						}
					}
					else
					{
						ResetEvent(pOverLapped->hEvent);
						AfxMessageBox(_T("*****USB接收数据失败，退出线程!"));
						return FALSE;
					}
				}
				else
				{
					for (int i = 0; i < nBytesRead; i++)
					{
						strTemp.Format("%.2X \n", byRecvBuf[i]);
						strRecv = strRecv + strTemp;
					}

					pDS->m_pMainDlg->m_editRecv.ReplaceSel(strRecv);

					TRACE("----------HID设备读操作成功并立即返回\n");
				}

				ResetEvent(pOverLapped->hEvent);  //准备下次接收
			}
			else
			{
				bRet = ReadFile(pDS->m_hUsbHid, byRecvBuf, 8, &nBytesRead, NULL);
				if (bRet && nBytesRead > 0)
				{
					for (int i = 0; i < nBytesRead; i++)
					{
						strTemp.Format("%.2X \n", byRecvBuf[i]);
						strRecv = strRecv + strTemp;
					}

					pDS->m_pMainDlg->m_editRecv.ReplaceSel(strRecv);
					//::SendMessage(pDS->m_pMainDlg->m_editRecv.m_hWnd, EM_REPLACESEL, FALSE, (LPARAM)strRecv.m_pszData);
				}

				Sleep(50);
			}
		}
	}

	return 0;
}

//启动线程
bool CUsbHIDRead::Start(HANDLE hUsbHid, CusbHIDDlg *pMainDlg)
{
	m_HWorkerThread = ::CreateThread(0, 0, _WorkerThread, (void *)m_pThreadParam, 0, &m_dwThreadID);
	if (m_HWorkerThread == NULL)
	{
		TRACE("CUsbHIDRead::Start() m_HWorkerThread create failed\n");
		return false;
	}

	m_hUsbHid = hUsbHid;
	this->m_pMainDlg = pMainDlg;
}

void CUsbHIDRead::Exit()
{
	if (m_HWorkerThread != NULL)  //说明线程还在运行
	{
		m_bThreadExit = true;
	}
}
