#pragma once
class CConvertStrHex
{
public:
	CConvertStrHex();
	~CConvertStrHex();
	static unsigned long  hex2str(unsigned char* data, TCHAR* buffer, unsigned long len);
	static unsigned long  str2hex(TCHAR* data, unsigned char* buffer, unsigned long len);
};

