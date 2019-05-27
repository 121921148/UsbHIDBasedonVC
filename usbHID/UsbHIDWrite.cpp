#include "StdAfx.h"
#include "UsbHIDWrite.h"
#include "usbHID.h"
#include "usbHIDDlg.h"

#define LED_NUM 4  //LED个数
#define LED1 0x01  //LED1
#define LED2 0x02  //LED2
#define LED3 0x03  //LED3
#define LED4 0x04  //LED4


CUsbHIDWrite::CUsbHIDWrite(void)
{
	m_HWorkerThread = INVALID_HANDLE_VALUE;
	m_pThreadParam = this;
	m_bThreadExit = false;
	m_strThreadName = "CGPSTcpserver";

	m_hUsbHid = INVALID_HANDLE_VALUE;
}


CUsbHIDWrite::~CUsbHIDWrite(void)
{
}

DWORD WINAPI CUsbHIDWrite::_WorkerThread(LPVOID lpParam)
{
	if (lpParam == NULL)
	{
		TRACE("CUsbHIDRead::_WorkerThread() 获取线程参数失败\n");
	}
	CUsbHIDWrite *pDS = (CUsbHIDWrite*)lpParam;

	TRACE("%s(%d) start\n", pDS->m_strThreadName, pDS->m_dwThreadID);

	unsigned char bySendBuf[8] = {0x00, 0x01, 0x00, 0x01, 0x08, 0x01, 0x91, 0x82};
	memset(bySendBuf, 0, sizeof(unsigned char) * 3);

	bool bAsynRW = pDS->m_pMainDlg->m_usbHID.m_bAsynRW;
	OVERLAPPED *pOverLapped = NULL;
	if (bAsynRW)
	{
		pOverLapped = &pDS->m_pMainDlg->m_usbHID.m_USBWriteOverlapped;
	}

	int nLedIndex = 1;  //0-表示全灭，非0-对应的LED亮
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
			BOOL bRet = FALSE;
			unsigned long nBytesWritten=0;

			if (bAsynRW)
			{
				if (nLedIndex == 0)
				{
					for (int i = 0; i < LED_NUM; i++)
					{
						bySendBuf[0] = LED1 + i;  //LED
						bySendBuf[1] = 0x00;  //灭

						ResetEvent(pOverLapped->hEvent);
						bRet = WriteFile(pDS->m_hUsbHid, bySendBuf, 8, &nBytesWritten, pOverLapped);
						if (!bRet)
						{
							if (ERROR_IO_PENDING == GetLastError())  //函数返回ERROR_IO_PENDING,表明USB正在进行读操作
							{
								TRACE("等待HID设备写操作.........\n");
								//使用WaitForSingleObject函数等待，直到读操作完成或延时已达到2秒钟.当USB读操作进行完毕后，USBReadOverlapped的hEvent事件会变为有信号
								if (WAIT_OBJECT_0 == WaitForSingleObject(pOverLapped->hEvent, 2000))
								{
									TRACE("完成HID设备写操作bRet = %d, nBytesWritten = %d\n", bRet, nBytesWritten);
								}
								else
								{
									TRACE("@@@HID设备写操作超时\n"); 
								}
								
							}
							else
							{
								ResetEvent(pOverLapped->hEvent);
								AfxMessageBox(_T("USB发送数据失败，退出线程!"));
								return FALSE;
							}
						}

						if (bRet && nBytesWritten > 0)
						{
							TRACE("Send successful nBytesWritten = %d\n", nBytesWritten);
						}

						ResetEvent(pOverLapped->hEvent);
					}
				}
				else
				{
					bySendBuf[0] = nLedIndex;  //LED
					bySendBuf[1] = 0x01;  //亮

					ResetEvent(pOverLapped->hEvent);
					bRet = WriteFile(pDS->m_hUsbHid, bySendBuf, 8, &nBytesWritten, pOverLapped);
					if (!bRet)
					{
						if (ERROR_IO_PENDING == GetLastError())  //函数返回ERROR_IO_PENDING,表明USB正在进行读操作
						{
							//使用WaitForSingleObject函数等待，直到读操作完成或延时已达到2秒钟.当USB读操作进行完毕后，USBReadOverlapped的hEvent事件会变为有信号
							if (WAIT_OBJECT_0 == WaitForSingleObject(pOverLapped->hEvent, 2000))
							{
								TRACE("$$$完成HID设备写操作bRet = %d, nBytesWritten = %d\n", bRet, nBytesWritten);
							}
							else
							{
								TRACE("---------------$$$HID设备写操作超时\n");
							}
						}
						else
						{
							ResetEvent(pOverLapped->hEvent);
							AfxMessageBox(_T("USB发送数据失败，退出线程!"));
							return FALSE;
						}
					}

					if (bRet && nBytesWritten > 0)
					{
						TRACE("Send successful nBytesWritten = %d\n", nBytesWritten);
					}

					ResetEvent(pOverLapped->hEvent);
				}
			}
			else
			{
				if (nLedIndex == 0)
				{
					for (int i = 0; i < LED_NUM; i++)
					{
						bySendBuf[0] = LED1 + i;  //LED
						bySendBuf[1] = 0x00;  //灭
						bRet = WriteFile(pDS->m_hUsbHid, bySendBuf, 8, &nBytesWritten, NULL);
						if (bRet && nBytesWritten > 0)
						{
							TRACE("Send successful nBytesWritten = %d\n", nBytesWritten);
						}
					}
				}
				else
				{
					bySendBuf[0] = nLedIndex;  //LED
					bySendBuf[1] = 0x01;  //亮
					bRet = WriteFile(pDS->m_hUsbHid, bySendBuf, 8, &nBytesWritten, NULL);
					if (bRet && nBytesWritten > 0)
					{
						TRACE("Send successful nBytesWritten = %d\n", nBytesWritten);
					}
				}
			}
		}

		if (nLedIndex == 4)
		{
			nLedIndex = 0;
		}
		else
		{
			nLedIndex++;
		}

		Sleep(1000);
	}

	return 0;
}

//启动线程
bool CUsbHIDWrite::Start(HANDLE hUsbHid, CusbHIDDlg *pMainDlg)
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

void CUsbHIDWrite::Exit()
{
	if (m_HWorkerThread != NULL)  //说明线程还在运行
	{
		m_bThreadExit = true;
	}
}
