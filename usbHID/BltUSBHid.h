#pragma once
#include<vector>  //ʹ��STL����
using std::vector;

//ʹ��hid.lib��setupapi.lib��ʱ������ͷ�ļ���
//����������Ҫͨ����Ŀ����-������-����-���������������ɣ�
//�����ɺ��ڽ��������Դ������-�ⲿ���������Զ������Щͷ�ļ��������С�
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
	GUID m_usbHIDGuid;  //HID�豸ID
	HDEVINFO m_pUsbDevInfo;  //HID�豸��Ϣ������ö�����е�HID�豸
	SP_DEVICE_INTERFACE_DATA m_usbInterfaceData;  //�豸�ӿ�����
	PSP_DEVICE_INTERFACE_DETAIL_DATA m_pHIdDetailData;  //�豸��ϸ��Ϣ
	int m_nHidDevs;  //ϵͳ��HID�豸��
	vector<PSP_DEVICE_INTERFACE_DETAIL_DATA> m_vecHIDInterfaceDetail;  //���ϵͳ��ö�ٵ�HID�豸�ӿڼ�
	HANDLE m_hCurOpenedHID;  //��ǰ���ӵ�HID�豸

	//��ȡ����HID�豸�б���֧�ִ�ָ��·����HID�豸�ӿ�
public:
	//����HID�б������豸
	bool InitHID(void);  //��ȡϵͳ����HID�豸�б�
	PHIDDetailList GetHIDList();  //�õ��Ի�ȡ���豸�б�
	HANDLE OpenHID(PCHAR   pchDevicePath, HIDD_ATTRIBUTES* pHIDDAttri = NULL);  //��ָ���豸����������

	//ֱ�Ӵ�ָ����HID�豸
	CString FindUSBDevice(CString Vid, CString Pid);  //Ѱ��ָ����HID�豸
	BOOL OpenUSBDevice(CString DeviceName, bool bAsyn = false);  //���ƶ���HID�豸
	HANDLE GetOpenedHIDH();  //��ȡ�Ѿ��򿪵�HID�豸���
	OVERLAPPED m_USBReadOverlapped;  //֧���첽��
	OVERLAPPED m_USBWriteOverlapped;  //֧���첽д
	bool m_bAsynRW;  //�첽����
};

