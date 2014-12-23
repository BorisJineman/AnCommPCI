#include "stdafx.h"
#include "ConvertStrHex.h"


CConvertStrHex::CConvertStrHex()
{
}


CConvertStrHex::~CConvertStrHex()
{
}


unsigned long CConvertStrHex::hex2str(unsigned char* data, TCHAR* buffer, unsigned long len)
{
	const TCHAR ascTable[17] = { _T("0123456789ABCDEF") };
	TCHAR*  tmp_p = buffer;
	unsigned long i, pos;
	pos = 0;

	for (i = 0; i < len; i++)
	{
		tmp_p[pos++] = ascTable[data[i] >> 4 & 0x0f];
		tmp_p[pos++] = ascTable[data[i] & 0x0f];
	}
	tmp_p[pos] = _T('\0');
	return pos;
}

unsigned long  CConvertStrHex::str2hex(TCHAR* data, unsigned char* buffer, unsigned long len)
{
	unsigned long i, j, tmp_len;
	TCHAR tmpData;
	for (i = 0; i < len; i++)
	{
		if ((data[i] >= _T('0')) && (data[i] <= _T('9')))
		{
			tmpData = data[i] - '0';
		}
		else if ((data[i] >= _T('A')) && (data[i] <= _T('F'))) //A....F  
		{
			tmpData = data[i] - 0x37;
		}
		else if ((data[i] >= _T('a')) && (data[i] <= _T('f'))) //a....f  
		{
			tmpData = data[i] - 0x57;
		}
		else{
			return -1;
		}
		data[i] = tmpData;
	}
	for (tmp_len = 0, j = 0; j < i; j += 2)
	{
		buffer[tmp_len++] = (data[j] << 4) | data[j + 1];
	}
	return tmp_len;
}

