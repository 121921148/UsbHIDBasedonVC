#include "StdAfx.h"
#include "BltUSBHid.h"


CBltUSBHid::CBltUSBHid(void)
{
	memset(&m_usbHIDGuid, 0, sizeof(GUID));
	memset(&m_usbInterfaceData, 0, sizeof(SP_DEVICE_INTERFACE_DATA));
	m_usbInterfaceData.cbSize=sizeof(SP_DEVICE_INTERFACE_DATA);
	m_pUsbDevInfo = NULL;  //需要使用API函数释放
	m_pHIdDetailData = NULL;
	m_nHidDevs = 0;
	m_hCurOpenedHID = INVALID_HANDLE_VALUE;  //使用完需要关闭句柄
	m_bAsynRW = false;
	memset(&m_USBReadOverlapped, 0, sizeof(OVERLAPPED));
	memset(&m_USBWriteOverlapped, 0, sizeof(OVERLAPPED));
}

CBltUSBHid::~CBltUSBHid(void)
{
	//释放获取到的HID设备接口列表
	int nSize = m_vecHIDInterfaceDetail.size();
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDevInterfaceDetail = NULL;
	for (int i = 0; i < nSize; i++)
	{
		pDevInterfaceDetail = m_vecHIDInterfaceDetail[i];
		delete pDevInterfaceDetail;
	}
	m_vecHIDInterfaceDetail.clear();

	//释放HID设备
	if (m_pUsbDevInfo)
	{
		SetupDiDestroyDeviceInfoList(m_pUsbDevInfo);
	}

	if (m_hCurOpenedHID)
	{
		CloseHandle(m_hCurOpenedHID);
		m_hCurOpenedHID = INVALID_HANDLE_VALUE;
	}

	//释放异步读写事件
	CloseHandle(m_USBReadOverlapped.hEvent);
	CloseHandle(m_USBWriteOverlapped.hEvent);
}

//初始化HID设备
bool CBltUSBHid::InitHID(void)
{
	//获取HID设备的GUID
	HidD_GetHidGuid(&m_usbHIDGuid);

	//获取HID设备信息
	m_pUsbDevInfo = SetupDiGetClassDevs(&m_usbHIDGuid, NULL, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if(m_pUsbDevInfo == INVALID_HANDLE_VALUE)
	{
		TRACE("获取HID设备信息失败: SetupDiGetClassDevs() failed!\n");
		return false;
	}

	//枚举所有HID设备（或接口）
	bool bEnumHIDInterface = false;  //枚举HID设备接口成功标识
	do
	{
		//枚举一次HID设备
		bEnumHIDInterface = SetupDiEnumDeviceInterfaces(
			                                         m_pUsbDevInfo,
												     NULL,
												     &m_usbHIDGuid,
												     m_nHidDevs,
												     &m_usbInterfaceData);
		
		//枚举成功一次就获取设备接口的详细信息
		//获得设备详细数据-1：获取设备详情接口指针，并分配空间
		DWORD dwSzie = 0, dwInterfaceDetailsize=0;
		if (bEnumHIDInterface)
		{
			SetupDiGetDeviceInterfaceDetail(
				m_pUsbDevInfo,
				&m_usbInterfaceData,
				NULL,
				0,
				&dwSzie,
				NULL);

			dwInterfaceDetailsize = dwSzie;
			m_pHIdDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(dwInterfaceDetailsize);
			if (m_pHIdDetailData)
			{
				m_pHIdDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			}
			else
			{
				bEnumHIDInterface  = false;
				continue;
			}

			//再次获得详细数据-2：获取设备详情内容
			if (!SetupDiGetDeviceInterfaceDetail(
				m_pUsbDevInfo,
				&m_usbInterfaceData,
				m_pHIdDetailData,
				dwSzie,
				&dwInterfaceDetailsize,
			    NULL))
			{
				bEnumHIDInterface  = false;
				continue;
			}

			//添加到容器中，获取HID设备的下个接口信息
			m_vecHIDInterfaceDetail.push_back(m_pHIdDetailData);
			m_pHIdDetailData = NULL;
			TRACE("第%d个 HID 设备：%s\n", m_nHidDevs, m_vecHIDInterfaceDetail[m_nHidDevs]->DevicePath);
			m_nHidDevs++;
			
			;  //获取HID设备路径：该路径可以文件方式打开相应的HID设备
		}
	}while(bEnumHIDInterface);

	if (m_nHidDevs == 0)  //说明没有获取到HID设备
	{
		return false;
	}

	return true;
}

//获取HID设备详情列表
PHIDDetailList CBltUSBHid::GetHIDList()
{
	if (m_vecHIDInterfaceDetail.empty())
	{
		return NULL;
	}
	else
	{
		return &m_vecHIDInterfaceDetail;
	}
}

//打开制定设备路径的HID设备
HANDLE CBltUSBHid::OpenHID(PCHAR   pinDevicePath, HIDD_ATTRIBUTES* poutHIDDAttri)
{
	 m_hCurOpenedHID = CreateFile(
						pinDevicePath,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						0,//FILE_ATTRIBUTE_NORMAL,
						NULL);

	 if (m_hCurOpenedHID == INVALID_HANDLE_VALUE)
	 {
		 return INVALID_HANDLE_VALUE;
	 }
	 else
	 {
		 //打开HID设备后获取其属性
		 if (false && !HidD_GetAttributes(m_hCurOpenedHID, poutHIDDAttri))
		 {
			 CloseHandle(m_hCurOpenedHID);

			 return INVALID_HANDLE_VALUE;
		 }

		 return m_hCurOpenedHID;
	 }
}

CString CBltUSBHid::FindUSBDevice(CString Vid, CString Pid)
{
	//查找本系统中HID类的GUID标识（系统中每类设备具有不同的GUID，由之可以获取某类设备信息）
	GUID guidHID;
	HidD_GetHidGuid(&guidHID);
	
	//查找符合HID规范的USB设备
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&guidHID, NULL, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hDevInfo == INVALID_HANDLE_VALUE)  //失败
	{
		return NULL;
	}
	
	int InterfaceIndex = 0;  //遍历HID接口的索引

	while (TRUE)
	{
		//查找USB设备接口
		SP_DEVICE_INTERFACE_DATA strtInterfaceData;
		strtInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);  //必须有这句，否则下边枚举会失败！

		//枚举HID接口，直到失败（TRUE表示成功）
		BOOL result = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guidHID, InterfaceIndex, &strtInterfaceData);
		if (!result)
		{
			break;
		}
		
		InterfaceIndex++;  //准备枚举下个HID接口

		//枚举成功一个设备（或接口）后，通过接口数据详情获取设备路径名
		DWORD dwDetailSize = 0;
		SP_DEVINFO_DATA strtDevInfoData;
		strtDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		//获取枚举后的设备接口详情（分两次调用SetupDiGetDeviceInterfaceDetail）
		//第一次调用：获得接口详细数据结构体的存储空间大小，并动态申请存储空间（调用会返回false）
	    result = SetupDiGetDeviceInterfaceDetail(hDevInfo, &strtInterfaceData, NULL, 0, &dwDetailSize, &strtDevInfoData);
		PSP_DEVICE_INTERFACE_DETAIL_DATA pstrtDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(dwDetailSize);  //此处必须用动态分配而不能定义局部变量，因为要在此结构体后边存放很大的内容，而局部变量没有如此大的空间！
		pstrtDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		//第二次调用：获取接口详细数据，并在申请的结构体中返回（需要判断是否调用成功）
		result = SetupDiGetDeviceInterfaceDetail(hDevInfo, &strtInterfaceData, pstrtDetailData, dwDetailSize, &dwDetailSize, &strtDevInfoData);
		if (result == FALSE)
		{
			free(pstrtDetailData);
			pstrtDetailData = NULL;
			break;
		}
		
		//根据接口数据详情获取设备路径
		TCHAR *pdevicePathName = pstrtDetailData->DevicePath;
		CString strDevicePathName = CString(pdevicePathName);
		free(pstrtDetailData);
		pstrtDetailData = NULL;
		pdevicePathName = NULL;

		//根据实参得到要打开设备路径的关键信息（子串："hid#vid_xxxx&pid_xxxx"）
		CString strSubPath = _T("hid#");
		strSubPath += _T("vid_") + Vid.MakeLower() + '&' + _T("pid_") + Pid.MakeLower();
		int idx = -1;
		idx = strDevicePathName.Find(strSubPath);
		if (idx >= 0)
		{
			//释放设备资源(hDevInfo是SetupDiGetClassDevs获取的)
			if (hDevInfo != INVALID_HANDLE_VALUE)
			{
				SetupDiDestroyDeviceInfoList(hDevInfo);
			}
			
			return strDevicePathName;
		}
	}  //End of while
    
	//释放设备资源(hDevInfo是SetupDiGetClassDevs获取的)
	if (hDevInfo != INVALID_HANDLE_VALUE)
	{
		SetupDiDestroyDeviceInfoList(hDevInfo);
	}
	
	return NULL;
}

BOOL CBltUSBHid::OpenUSBDevice(CString DeviceName, bool bAsyn)
{
	//为HID设备创建读写文件句柄
	//制定异步标识FILE_FLAG_OVERLAPPED时，Read和Write也要使用异步读写模式，否则读写失败；反之为0时，读写使用同步模式

	DWORD dwFlagsAndAttr = 0;
	if (bAsyn )
	{
		m_bAsynRW = true;
		dwFlagsAndAttr = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;

		memset(&m_USBReadOverlapped, 0, sizeof(OVERLAPPED));
		m_USBReadOverlapped.hEvent = CreateEvent(NULL, //指向SECURITY_ATTRIBUTES结构体，此结构体决定函数的返回句柄是否可以让子进程继承。如果这个参数为NULL，这个句柄是不能继承的
		  TRUE, //指定将创建的EVENT是自动复位还是手动复位。如果为TRUE，需要用ResetEvent(HANDLE)函数手动复位状态为无信号，即一旦该EVENT被设置成有信号，则它会一直等到ResetEvent调用时才为无信号状态。如果为FALSE，当一个有信号的等待线程被释放后，系统会自动复位状态为无信号状态
		  FALSE, //指定事件对象的初始状态。如果为TRUE，初始状态为有信号，否则为无信号
		  NULL //如果为NULL，创建一个没有名称的事件
		  );

		memset(&m_USBWriteOverlapped, 0, sizeof(OVERLAPPED));
		m_USBWriteOverlapped.hEvent = CreateEvent(NULL, //指向SECURITY_ATTRIBUTES结构体，此结构体决定函数的返回句柄是否可以让子进程继承。如果这个参数为NULL，这个句柄是不能继承的
		  TRUE, //指定将创建的EVENT是自动复位还是手动复位。如果为TRUE，需要用ResetEvent(HANDLE)函数手动复位状态为无信号，即一旦该EVENT被设置成有信号，则它会一直等到ResetEvent调用时才为无信号状态。如果为FALSE，当一个有信号的等待线程被释放后，系统会自动复位状态为无信号状态
		  FALSE, //指定事件对象的初始状态。如果为TRUE，初始状态为有信号，否则为无信号
		  NULL //如果为NULL，创建一个没有名称的事件
		  );
	}
	else
	{
		dwFlagsAndAttr = 0;
		m_bAsynRW = false;
	}

	m_hCurOpenedHID = CreateFile(DeviceName, 
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING, 
		dwFlagsAndAttr,  //文件属性与标志,用于指定USB是否进行异步操作，该值为FILE_FLAG_OVERLAPPED表示使用异步的I/O，该值为0表示同步I/O操作
		NULL
		);
	if (m_hCurOpenedHID == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	//设置HID设备输入缓冲的大小
	int NumInputBuffers = 8;
	BOOL bRet = HidD_SetNumInputBuffers(m_hCurOpenedHID, NumInputBuffers);
	if(!bRet)
	{
		return FALSE;
	}

	//获取HID预分析数据
	PHIDP_PREPARSED_DATA PreparsedData;
	bRet = HidD_GetPreparsedData(m_hCurOpenedHID, &PreparsedData);
	if(!bRet)
	{
		return FALSE;
	}
	
	//获取设备容量
	unsigned short InputReportLength;
	unsigned short OutputReportLength;
	HIDP_CAPS Capabilities;
	HidP_GetCaps(PreparsedData, &Capabilities);
	InputReportLength = Capabilities.InputReportByteLength;
	OutputReportLength = Capabilities.OutputReportByteLength;

	//释放HidD_GetPreparsedData使用的资源
	HidD_FreePreparsedData(PreparsedData);

	return TRUE;
}

HANDLE CBltUSBHid::GetOpenedHIDH()
{
	return m_hCurOpenedHID;
}