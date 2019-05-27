#include "StdAfx.h"
#include "UsbHIDWrite.h"
#include "usbHID.h"
#include "usbHIDDlg.h"

#define LED_NUM 4  //LED����
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
		TRACE("CUsbHIDRead::_WorkerThread() ��ȡ�̲߳���ʧ��\n");
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

	int nLedIndex = 1;  //0-��ʾȫ�𣬷�0-��Ӧ��LED��
	while(true)
	{
		if (pDS->m_bThreadExit)
		{
			//�߳��˳�������
			pDS->m_bThreadExit = false;
			CloseHandle(pDS->m_HWorkerThread);
			pDS->m_HWorkerThread = INVALID_HANDLE_VALUE;

			TRACE("%s(%d) exit\n", pDS->m_strThreadName, pDS->m_dwThreadID);

			break;  //�˳��̺߳�������ֹ�߳�
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
						bySendBuf[1] = 0x00;  //��

						ResetEvent(pOverLapped->hEvent);
						bRet = WriteFile(pDS->m_hUsbHid, bySendBuf, 8, &nBytesWritten, pOverLapped);
						if (!bRet)
						{
							if (ERROR_IO_PENDING == GetLastError())  //��������ERROR_IO_PENDING,����USB���ڽ��ж�����
							{
								TRACE("�ȴ�HID�豸д����.........\n");
								//ʹ��WaitForSingleObject�����ȴ���ֱ����������ɻ���ʱ�Ѵﵽ2����.��USB������������Ϻ�USBReadOverlapped��hEvent�¼����Ϊ���ź�
								if (WAIT_OBJECT_0 == WaitForSingleObject(pOverLapped->hEvent, 2000))
								{
									TRACE("���HID�豸д����bRet = %d, nBytesWritten = %d\n", bRet, nBytesWritten);
								}
								else
								{
									TRACE("@@@HID�豸д������ʱ\n"); 
								}
								
							}
							else
							{
								ResetEvent(pOverLapped->hEvent);
								AfxMessageBox(_T("USB��������ʧ�ܣ��˳��߳�!"));
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
					bySendBuf[1] = 0x01;  //��

					ResetEvent(pOverLapped->hEvent);
					bRet = WriteFile(pDS->m_hUsbHid, bySendBuf, 8, &nBytesWritten, pOverLapped);
					if (!bRet)
					{
						if (ERROR_IO_PENDING == GetLastError())  //��������ERROR_IO_PENDING,����USB���ڽ��ж�����
						{
							//ʹ��WaitForSingleObject�����ȴ���ֱ����������ɻ���ʱ�Ѵﵽ2����.��USB������������Ϻ�USBReadOverlapped��hEvent�¼����Ϊ���ź�
							if (WAIT_OBJECT_0 == WaitForSingleObject(pOverLapped->hEvent, 2000))
							{
								TRACE("$$$���HID�豸д����bRet = %d, nBytesWritten = %d\n", bRet, nBytesWritten);
							}
							else
							{
								TRACE("---------------$$$HID�豸д������ʱ\n");
							}
						}
						else
						{
							ResetEvent(pOverLapped->hEvent);
							AfxMessageBox(_T("USB��������ʧ�ܣ��˳��߳�!"));
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
						bySendBuf[1] = 0x00;  //��
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
					bySendBuf[1] = 0x01;  //��
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

//�����߳�
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
	if (m_HWorkerThread != NULL)  //˵���̻߳�������
	{
		m_bThreadExit = true;
	}
}
