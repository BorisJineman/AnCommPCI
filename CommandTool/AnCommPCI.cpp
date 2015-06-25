


#include "stdafx.h"

#include "AnCommPCI.h"

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
	unsigned char *buff = (unsigned char *)malloc(0x40000);

	ReadCmd cmd;
	cmd.offset = 0;	
	cmd.length = len;

	DeviceIoControl(m_hDevice, IOCTL_BEGIN_RECEIVE_DATA, &cmd, sizeof(ReadCmd), NULL, 0, &retLen, NULL);

	do
	{
		Sleep(1);
		DeviceIoControl(m_hDevice, IOCTL_RECEIVE_IGNORE_LEN_DATA, NULL, 0, (LPVOID)buff, cmd.length, &retLen, NULL);

	} while (retLen == 0);

	memcpy(data + cmd.offset, buff, retLen);

	free(buff);
	return retLen;
}

void CAnCommPCI::ReceiveAsFile()
{
	

	unsigned long retLen = 0;
	unsigned long receiveDataLen = 0;
	unsigned char *buff = (unsigned char *)malloc(0x40000);
	if (!m_bFileReady)
	{
		ReadCmd cmd;
		cmd.offset = 0;
		cmd.length = 0x40000;
		m_hCurrentCMDStatus = cmd;
	}

	do
	{
		DeviceIoControl(m_hDevice, IOCTL_BEGIN_RECEIVE_DATA, &m_hCurrentCMDStatus, sizeof(ReadCmd), NULL, 0, &retLen, NULL);
		Sleep(1);
		DeviceIoControl(m_hDevice, IOCTL_RECEIVE_DATA, NULL, 0, (LPVOID)buff, m_hCurrentCMDStatus.length, &receiveDataLen, NULL);
		if (receiveDataLen != 0)
		{
			if (!m_bFileReady)
			{
				
				SYSTEMTIME sys;
				GetLocalTime(&sys);
				CString fileName;
				fileName.Format(_T("%4d%02d%02d_%02d%02d%02d_%03d.bin"), sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
				CString fileFullName(get_FileSavePath());
				fileFullName.Append(_T("\\"));
				fileFullName.Append(fileName);
				if (m_hFile.Open(fileFullName, CFile::modeWrite | CFile::modeCreate))
				{
					m_bFileReady = true;
				}
				else
				{
					break;
				}

			}
			if (m_bFileReady)
			{
				m_hFile.Write(buff, receiveDataLen);
			}

			m_hCurrentCMDStatus.offset += 0x40000;
			//waitTimes = 0;
			
		}

		if (m_bFileReady)
		{
			unsigned long receiveFinished = 0;
			DeviceIoControl(m_hDevice, IOCTL_CHECK_RECEIVE_FINISHED, NULL, 0, (LPVOID)&receiveFinished, sizeof(unsigned long), &retLen, NULL);
			if (receiveFinished)
			{
				m_hFile.Flush();
				m_hFile.Close();
				m_bFileReady = false;
			}
		}
		
	} while (false && receiveDataLen != 0);

	//waitTimes++;
	//if (m_bFileReady&&waitTimes >= 3)
	//{
	//	m_hFile.Flush();
	//	m_hFile.Close();
	//	m_bFileReady = false;
	//}
	
	free(buff);
	
	return;
	
}


void CAnCommPCI::GetCurrentInfo(DeviceStatus * status)
{
	unsigned long retLen = 0;
	DeviceIoControl(m_hDevice, IOCTL_GET_DEVICE_STATUS, NULL, 0, status, sizeof(DeviceStatus), &retLen, NULL);
}


void CAnCommPCI::ResetComm(unsigned long operation)
{
	unsigned long retLen = 0;

	if (operation == 1)
		DeviceIoControl(m_hDevice, IOCTL_RESET_COMM, NULL, 0, NULL, 0, &retLen, NULL);
	else if (operation == 0)
		DeviceIoControl(m_hDevice, IOCTL_RESET_COMM_2, NULL, 0, NULL, 0, &retLen, NULL);

}