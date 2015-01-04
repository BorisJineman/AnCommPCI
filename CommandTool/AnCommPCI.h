#pragma once

#include "..\AnCommPCI\guid.h"
#include "..\AnCommPCI\Ioctls.h"

class CAnCommPCI
{
public:


	CAnCommPCI();

	~CAnCommPCI()
	{
		if (m_lStatus != 0)
		{
			CloseDevice();
		}
	}

	static CAnCommPCI * GetInstance();
	
	unsigned long OpenDevice();

	void CloseDevice();

	void Send(unsigned char* data, unsigned long len);
	unsigned long Receive(unsigned char* data, unsigned long len);

	void ReceiveAsFile();
	void GetCurrentInfo(DeviceStatus * status);


	void ResetComm();

	CString get_FileSavePath(){ return m_sFileSavePath; }
	void set_FileSavePath(CString value){ m_sFileSavePath = value; }
		
private:

	
	static CAnCommPCI * m_pInstance;
	
	unsigned long m_lStatus;
	HANDLE m_hDevice;

	HANDLE GetDeviceViaInterface(GUID* pGuid, DWORD instance);

	CString m_sFileSavePath;

	CFile m_hFile;
	ReadCmd m_hCurrentCMDStatus;
	bool m_bFileReady;
	//unsigned long waitTimes;


	
};

