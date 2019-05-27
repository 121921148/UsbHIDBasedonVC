
// usbHIDDlg.h : 头文件
//

#pragma once
#include "BltUSBHid.h"
#include "UsbHIDRead.h"
#include "UsbHIDWrite.h"


// CusbHIDDlg 对话框
class CusbHIDDlg : public CDialogEx
{
// 构造
public:
	CusbHIDDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_USBHID_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_listDevice;

public:
	CBltUSBHid m_usbHID;  //HID设备操作类
	HIDD_ATTRIBUTES m_HIDDAttr;  //HID设备属性
	HANDLE m_hHIDOpened;  //HID设备的句柄

	//线程操作
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
