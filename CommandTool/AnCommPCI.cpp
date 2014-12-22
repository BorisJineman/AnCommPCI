


#include "stdafx.h"

#include "AnCommPCI.h"
#include "..\AnCommPCI\guid.h"
#include "..\AnCommPCI\Ioctls.h"

CAnCommPCI* CAnCommPCI::m_pInstance = NULL;

CAnCommPCI::CAnCommPCI()
{
}

CAnCommPCI * CAnCommPCI::GetInstance()
{
	if (m_pInstance == NULL)
		m_pInstance = new CAnCommPCI();
	return m_pInstance;
}

HANDLE CAnCommPCI::GetDeviceViaInterface(GUID* pGuid, DWORD instance)
{
	// Get handle to relevant device information set
	HDEVINFO info = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if (info == INVALID_HANDLE_VALUE)
	{
		//printf("No HDEVINFO available for this GUID\n");
		return NULL;
	}


	// Get interface data for the requested instance
	SP_INTERFACE_DEVICE_DATA ifdata;
	ifdata.cbSize = sizeof(ifdata);
	if (!SetupDiEnumDeviceInterfaces(info, NULL, pGuid, instance, &ifdata))
	{
		//printf("No SP_INTERFACE_DEVICE_DATA available for this GUID instance\n");
		SetupDiDestroyDeviceInfoList(info);
		return NULL;
	}

	// Get size of symbolic link name
	DWORD ReqLen;
	SetupDiGetDeviceInterfaceDetail(info, &ifdata, NULL, 0, &ReqLen, NULL);
	PSP_INTERFACE_DEVICE_DETAIL_DATA ifDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)(new char[ReqLen]);
	if (ifDetail == NULL)
	{
		SetupDiDestroyDeviceInfoList(info);
		return NULL;
	}

	// Get symbolic link name
	ifDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	if (!SetupDiGetDeviceInterfaceDetail(info, &ifdata, ifDetail, ReqLen, NULL, NULL))
	{
		SetupDiDestroyDeviceInfoList(info);
		delete ifDetail;
		return NULL;
	}

	//printf("Symbolic link is %s\n", ifDetail->DevicePath);
	// Open file
	HANDLE rv = CreateFile(ifDetail->DevicePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (rv == INVALID_HANDLE_VALUE) rv = NULL;

	delete ifDetail;
	SetupDiDestroyDeviceInfoList(info);

	return rv;
}

unsigned long CAnCommPCI::OpenDevice()
{
	unsigned long len;
	m_hDevice=GetDeviceViaInterface((LPGUID)&MY_WDM_DEVICE, 0);
	DeviceIoControl(m_hDevice, IOCTL_ALLOC_MEM, NULL, 0,NULL, 0, &len, NULL);
	if (m_hDevice == NULL)
	{
		m_lStatus = 0;
		return 0;
	}
	else
	{
		m_lStatus = 1;
		return 1;
	}
}

void CAnCommPCI::CloseDevice()
{	
	CloseHandle(m_hDevice);
	m_hDevice = NULL;
	m_lStatus = 0;
}


void CAnCommPCI::Send(unsigned char* data, unsigned long len)
{
	DeviceIoControl(m_hDevice, IOCTL_SEND_DATA, (LPVOID)data, len, (LPVOID)data, 0, &len, NULL);
}

unsigned long CAnCommPCI::Receive(unsigned char* data, unsigned long len)
{

	unsigned long retLen = 0;
	unsigned char *buff = (unsigned char *)malloc(0x20000);
	ReadCmd cmd;
	int remainLen = len;
	cmd.offset = 0;
	

	if (remainLen > 0)
	{
		if (remainLen > 0x20000)
		{
			cmd.length = 0x20000;
		}
		else
		{
			cmd.length = remainLen;
		}
			DeviceIoControl(m_hDevice, IOCTL_BEGIN_RECEIVE_DATA, &cmd, sizeof(ReadCmd), NULL, 0, &retLen, NULL);

			do
			{
				Sleep(1);
				DeviceIoControl(m_hDevice, IOCTL_RECEIVE_DATA, NULL, 0, (LPVOID)buff, cmd.length, &retLen, NULL);

			} while (retLen == 0);

			memcpy(data + cmd.offset, buff, retLen);
			cmd.offset += retLen;
			remainLen -= retLen;

		
		
	}

	free(buff);
	return len;
}