
// usbHIDDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "usbHID.h"
#include "usbHIDDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CusbHIDDlg 对话框




CusbHIDDlg::CusbHIDDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CusbHIDDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CusbHIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DEVICE, m_listDevice);
	DDX_Control(pDX, IDC_EDIT_VID, m_editVID);
	DDX_Control(pDX, IDC_EDIT_PID, m_editPID);
	DDX_Control(pDX, IDC_EDIT_VER, m_editVER);
	DDX_Control(pDX, IDC_EDIT_PATH, m_editPath);
	DDX_Control(pDX, IDC_EDIT_RECV, m_editRecv);
	DDX_Control(pDX, IDC_EDIT_VID_TOOPEN, m_editVIDToOpen);
	DDX_Control(pDX, IDC_EDIT_PID_TOOPEN, m_editPIDToOpen);
	DDX_Control(pDX, IDC_CHECK3, m_checkRWType);
}

BEGIN_MESSAGE_MAP(CusbHIDDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SCAN, &CusbHIDDlg::OnBnClickedButtonScan)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CusbHIDDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_START_RECV, &CusbHIDDlg::OnBnClickedButtonStartRecv)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CusbHIDDlg::OnBnClickedButtonSend)
	ON_WM_TIMER()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CusbHIDDlg 消息处理程序

BOOL CusbHIDDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitDlgCtrl();  //控件、变量初始化
	if (!m_usbHID.InitHID())
	{
		TRACE("HID设备初始化失败: CBltUSBHid::InitHID(void) failed!\n");
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CusbHIDDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CusbHIDDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CusbHIDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//初始化控件
void CusbHIDDlg::InitDlgCtrl()
{
	//HID相关变量初始化
	memset(&m_HIDDAttr, 0, sizeof(HIDD_ATTRIBUTES));
	m_hHIDOpened = INVALID_HANDLE_VALUE;

	//初始化devicelist控件
	int i;  
	LV_COLUMN lvcolumn;
	TCHAR rgtsz[2][10] = {"序号", "设备描述"};
	int ncolumnWidth[2] = {40, 320};
	for (i = 0; i < 2; i++)  // add the columns to the list control
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcolumn.fmt = LVCFMT_CENTER;
		lvcolumn.pszText = rgtsz[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = ncolumnWidth[i];
		m_listDevice.InsertColumn(i, &lvcolumn);  // assumes return value is OK.
	}
	DWORD dwStyle = ListView_GetExtendedListViewStyle(m_listDevice);
	//Add the full row select and grid line style to the existing extended styles
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP ;
	ListView_SetExtendedListViewStyle (m_listDevice, dwStyle);
	//初始化devicelist控件-END

	/*------------------------------------------------------------------
	CEdit 控件写满一行自动换行的属性设置：
	必须设置Vertical Scrollbar = TRUE
	必须设置MultiLine = TRUE
	必须设置Horizontal Scrollbar = FALSE
	必须设置Auto HScroll = FALSE
	可选设置Want Return = FALSE  
	可选设置Auto VScroll = TRUE
	------------------------------------------------------------------ */
	m_editRecv.SetSel(-1, -1);  //接收区edit控件自动滚屏，并设置为可以显示多行

	m_editVIDToOpen.SetWindowText("0483");
	m_editPIDToOpen.SetWindowText("5750");
}

void CusbHIDDlg::OnBnClickedButtonScan()
{
	// TODO: 在此添加控件通知处理程序代码
	PHIDDetailList pHidDetailList = NULL;

	pHidDetailList = m_usbHID.GetHIDList();
	if (pHidDetailList)
	{
		m_listDevice.DeleteAllItems();
		int nLines = pHidDetailList->size();
		if (nLines > 0)
		{
			for (int i = 0; i < nLines; i++)
			{
				CString strIndex, strPath;

				strIndex.Format("%d", i);
				m_listDevice.InsertItem(nLines, strIndex);
				strIndex.Format("%s", pHidDetailList->at(i)->DevicePath);
				m_listDevice.SetItemText(i, 1, strIndex);
			}
		}
	}
	else
	{
		MessageBox("HID扫描失败", "提醒", MB_OK);
	}
}


void CusbHIDDlg::OnBnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR szBuf[1024] = {0};
	LVITEM lvi;
	bool bAsynRW = false;  //m_checkRWType.GetCheck();
	int nIdx=m_listDevice.GetSelectionMark();
	if (nIdx<0)  //设备列表中没有设备或没有选中设备时检查是否有默认打开设备，并打开
	{
		bAsynRW = true;  //直接打开指定设备时，默认使用异步读写
		CString strPID, strVID, strPath;
		m_editVIDToOpen.GetWindowText(strVID);
		m_editPIDToOpen.GetWindowText(strPID);
		if (!strVID.IsEmpty() && !strPID.IsEmpty())
		{
			strPath = m_usbHID.FindUSBDevice(strVID, strPID);
			if (!strPath.IsEmpty())
			{
				//打开HID设备
				if (m_usbHID.OpenUSBDevice(strPath, bAsynRW))
				{
					m_hHIDOpened = m_usbHID.GetOpenedHIDH();
				}
				else
				{
					AfxMessageBox(_T("打开指定设备失败")); 
				}
			}
			else
			{
				AfxMessageBox(_T("找不指定的HID设备")); 
			}
		}
		else
		{
			AfxMessageBox(_T("没有可连接的HID设备"));
		}
		
	} 
	else  //打开选中的HID设备
	{
		//获取选中行文本
		lvi.iItem = nIdx;
		lvi.iSubItem = 1;
		lvi.mask = LVIF_TEXT;
		lvi.pszText = szBuf;
		lvi.cchTextMax = 1024;
		m_listDevice.GetItem(&lvi);

		m_HIDDAttr.Size=sizeof(HIDD_ATTRIBUTES);
		m_hHIDOpened = m_usbHID.OpenHID(szBuf, &m_HIDDAttr);
		if (m_hHIDOpened == INVALID_HANDLE_VALUE)
		{
			AfxMessageBox(_T("打开选中HID设备失败"));  
		}
		else
		{
			//获取HID设备属性在界面显示
			CString str;
			str.Format("0x%.4X", m_HIDDAttr.VendorID);
			m_editVID.SetWindowTextA(str);
			str.Format("0x%.4X", m_HIDDAttr.ProductID);
			m_editPID.SetWindowTextA(str);
			str.Format("0x%.4X", m_HIDDAttr.VersionNumber);
			m_editVER.SetWindowTextA(str);
			m_editPath.SetWindowTextA(szBuf);
		}
	}
}


void CusbHIDDlg::OnBnClickedButtonStartRecv()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_hHIDOpened != INVALID_HANDLE_VALUE)
	{
		m_HIDReadThread.Start(m_hHIDOpened, this);

		SetTimer(1, 2000, NULL);  //定时清空接收区内容
	}
	else
	{
		AfxMessageBox(_T("HID设备句柄无效！"));  
	}
}


void CusbHIDDlg::OnBnClickedButtonSend()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_hHIDOpened != INVALID_HANDLE_VALUE)
	{
		m_HIDWriteThread.Start(m_hHIDOpened, this);
	}
	else
	{
		AfxMessageBox(_T("HID设备句柄无效！"));  
	}
}


void CusbHIDDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int iLineNum = m_editRecv.GetLineCount();
    if (iLineNum > 200)
	{
		m_editRecv.SetSel(0, -1);
		m_editRecv.Clear();
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CusbHIDDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//退出线程
	if (m_HIDReadThread.m_HWorkerThread != INVALID_HANDLE_VALUE)
	{
		m_HIDReadThread.Exit();
	}
	if (m_HIDWriteThread.m_HWorkerThread != INVALID_HANDLE_VALUE)
	{
		m_HIDWriteThread.Exit();
	}

	//退出定时器
	KillTimer(1);

	CDialogEx::OnClose();
}
