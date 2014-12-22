#pragma once
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
		
private:

	static CAnCommPCI * m_pInstance;
	
	unsigned long m_lStatus;
	HANDLE m_hDevice;

	HANDLE GetDeviceViaInterface(GUID* pGuid, DWORD instance);
	
};

