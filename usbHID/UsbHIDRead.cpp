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
		TRACE("CUsbHIDRead::_WorkerThread() ��ȡ�̲߳���ʧ��\n");
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
			//�߳��˳�������
			pDS->m_bThreadExit = false;
			CloseHandle(pDS->m_HWorkerThread);
			pDS->m_HWorkerThread = INVALID_HANDLE_VALUE;

			TRACE("%s(%d) exit\n", pDS->m_strThreadName, pDS->m_dwThreadID);

			break;  //�˳��̺߳�������ֹ�߳�
		}

		if (pDS->m_hUsbHid != INVALID_HANDLE_VALUE)
		{
			unsigned char byRecvBuf[20];
			BOOL bRet = FALSE;
			unsigned long nBytesRead=0;

			if (bAsynRW)  //�첽��д
			{
				ResetEvent(pOverLapped->hEvent);
				bRet = ReadFile(pDS->m_hUsbHid, byRecvBuf, 8, &nBytesRead, pOverLapped);

				if (!bRet)
				{
					if (ERROR_IO_PENDING == GetLastError()) //��������ERROR_IO_PENDING,����USB���ڽ��ж�����
					{
						TRACE("�ȴ�HID�豸������....\n");
						//ʹ��WaitForSingleObject�����ȴ���ֱ����������ɻ���ʱ�Ѵﵽ2����.��USB������������Ϻ�USBReadOverlapped��hEvent�¼����Ϊ���ź�
						if (WAIT_OBJECT_0 == WaitForSingleObject(pOverLapped->hEvent, 2000))
						{
							for (int i = 0; i < 8; i++)
							{
								strTemp.Format("%.2X \n", byRecvBuf[i]);
								strRecv = strRecv + strTemp;
							}

							pDS->m_pMainDlg->m_editRecv.ReplaceSel(strRecv);
							TRACE("���HID�豸������!!!\n");
						}
						else
						{
							TRACE("*****HID�豸������------------��ʱ\n");
						}
					}
					else
					{
						ResetEvent(pOverLapped->hEvent);
						AfxMessageBox(_T("*****USB��������ʧ�ܣ��˳��߳�!"));
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

					TRACE("----------HID�豸�������ɹ�����������\n");
				}

				ResetEvent(pOverLapped->hEvent);  //׼���´ν���
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

//�����߳�
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
	if (m_HWorkerThread != NULL)  //˵���̻߳�������
	{
		m_bThreadExit = true;
	}
}
