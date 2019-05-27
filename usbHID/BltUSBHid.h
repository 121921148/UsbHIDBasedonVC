#pragma once
#include<vector>  //使用STL容器
using std::vector;

//使用hid.lib，setupapi.lib库时包含的头文件；
//该两个库需要通过项目属性-链接器-输入-附加依赖库添加完成；
//添加完成后，在解决方案资源管理器-外部依赖项中自动添加这些头文件到工程中。
extern "C" 
{
#include "hidsdi.h" 
#include "setupapi.h"
}

typedef vector<PSP_DEVICE_INTERFACE_DETAIL_DATA>* PHIDDetailList;

class CBltUSBHid
{
public:
	CBltUSBHid(void);
	~CBltUSBHid(void);

private:
	GUID m_usbHIDGuid;  //HID设备ID
	HDEVINFO m_pUsbDevInfo;  //HID设备信息，可以枚举所有的HID设备
	SP_DEVICE_INTERFACE_DATA m_usbInterfaceData;  //设备接口数据
	PSP_DEVICE_INTERFACE_DETAIL_DATA m_pHIdDetailData;  //设备详细信息
	int m_nHidDevs;  //系统中HID设备数
	vector<PSP_DEVICE_INTERFACE_DETAIL_DATA> m_vecHIDInterfaceDetail;  //存放系统中枚举的HID设备接口集
	HANDLE m_hCurOpenedHID;  //当前连接的HID设备

	//获取所有HID设备列表，并支持打开指定路径的HID设备接口
public:
	//搜索HID列表，并打开设备
	bool InitHID(void);  //获取系统所有HID设备列表
	PHIDDetailList GetHIDList();  //得到以获取的设备列表
	HANDLE OpenHID(PCHAR   pchDevicePath, HIDD_ATTRIBUTES* pHIDDAttri = NULL);  //打开指定设备并返回属性

	//直接打开指定的HID设备
	CString FindUSBDevice(CString Vid, CString Pid);  //寻找指定的HID设备
	BOOL OpenUSBDevice(CString DeviceName, bool bAsyn = false);  //打开制定的HID设备
	HANDLE GetOpenedHIDH();  //获取已经打开的HID设备句柄
	OVERLAPPED m_USBReadOverlapped;  //支持异步读
	OVERLAPPED m_USBWriteOverlapped;  //支持异步写
	bool m_bAsynRW;  //异步操作
};

