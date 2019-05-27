
// usbHIDDlg.h : ͷ�ļ�
//

#pragma once
#include "BltUSBHid.h"
#include "UsbHIDRead.h"
#include "UsbHIDWrite.h"


// CusbHIDDlg �Ի���
class CusbHIDDlg : public CDialogEx
{
// ����
public:
	CusbHIDDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_USBHID_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_listDevice;

public:
	CBltUSBHid m_usbHID;  //HID�豸������
	HIDD_ATTRIBUTES m_HIDDAttr;  //HID�豸����
	HANDLE m_hHIDOpened;  //HID�豸�ľ��

	//�̲߳���
public:
	CUsbHIDRead m_HIDReadThread;
	CUsbHIDWrite m_HIDWriteThread;

private:
	void InitDlgCtrl();
public:
	afx_msg void OnBnClickedButtonScan();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonStartRecv();
	CEdit m_editVID;
	CEdit m_editPID;
	CEdit m_editVER;
	CEdit m_editPath;	
	CEdit m_editRecv;
	afx_msg void OnBnClickedButtonSend();
	CEdit m_editVIDToOpen;
	CEdit m_editPIDToOpen;
	CButton m_checkRWType;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
};
