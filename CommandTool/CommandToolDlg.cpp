
// CommandToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CommandTool.h"
#include "CommandToolDlg.h"
#include "afxdialogex.h"
#include "AnCommPCI.h"
#include "..\AnCommPCI\Ioctls.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CCommandToolDlg dialog



CCommandToolDlg::CCommandToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCommandToolDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCommandToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICE_STATUS, m_DeviceStatus);
}

BEGIN_MESSAGE_MAP(CCommandToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CCommandToolDlg::OnOpenDeviceBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON2, &CCommandToolDlg::OnCloseDeviceBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON9, &CCommandToolDlg::OnStartBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON10, &CCommandToolDlg::OnEndBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON4, &CCommandToolDlg::OnReceiveBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON5, &CCommandToolDlg::OnSendAFileBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON6, &CCommandToolDlg::OnSetFilePathBtnClicked)
	ON_WM_TIMER()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON7, &CCommandToolDlg::OnExitBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON8, &CCommandToolDlg::OnResetCommBtnClicked)
END_MESSAGE_MAP()


// CCommandToolDlg message handlers

BOOL CCommandToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here


	SetDlgItemText(IDC_EDIT1, _T("10"));
	SetDlgItemText(IDC_EDIT2, _T("13"));
	SetDlgItemText(IDC_EDIT3, _T("13"));
	SetDlgItemText(IDC_EDIT4, _T("1000"));
	SetDlgItemText(IDC_EDIT5, _T("2"));
	SetDlgItemText(IDC_EDIT6, _T("65535"));
	SetDlgItemText(IDC_EDIT7, _T("2"));
	SetDlgItemText(IDC_EDIT8, _T("10"));

	
	((CComboBox*)GetDlgItem(IDC_COMBO2))->AddString(_T("功放自激励"));
	((CComboBox*)GetDlgItem(IDC_COMBO2))->AddString(_T("波表激励"));
	((CComboBox*)GetDlgItem(IDC_COMBO2))->SetCurSel(0);

	SetDlgItemText(IDC_EDIT10, _T("100"));
	SetDlgItemText(IDC_EDIT11, _T("10"));

	((CComboBox*)GetDlgItem(IDC_COMBO1))->AddString(_T("内触发"));
	((CComboBox*)GetDlgItem(IDC_COMBO1))->AddString(_T("软件触发"));
	((CComboBox*)GetDlgItem(IDC_COMBO1))->AddString(_T("外触发"));
	((CComboBox*)GetDlgItem(IDC_COMBO1))->SetCurSel(0);



	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCommandToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCommandToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCommandToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCommandToolDlg::OnOpenDeviceBtnClicked()
{
	// TODO: Add your control notification handler code here
	if (CAnCommPCI::GetInstance()->OpenDevice())
	{
		m_DeviceStatus.SetWindowText(_T("打开"));
		SetTimer(GET_DEVICE_CURRENT_INFO_TIMER, 50, NULL);
	}
	else
	{
		MessageBox(_T("打开设备失败."));
	}
	
}


void CCommandToolDlg::OnCloseDeviceBtnClicked()
{
	// TODO: Add your control notification handler code here
	KillTimer(GET_DEVICE_CURRENT_INFO_TIMER);
	CAnCommPCI::GetInstance()->CloseDevice();
	m_DeviceStatus.SetWindowText(_T("关闭"));

}


void CCommandToolDlg::OnStartBtnClicked()
{
	// TODO: Add your control notification handler code here

	if (CAnCommPCI::GetInstance()->get_FileSavePath().IsEmpty())
	{
		MessageBox(_T("未设置文件存储路径."));
	}


	CString str;
	unsigned char* pBuffer = (unsigned char*)malloc(1024);
	unsigned int temp = 0;
	unsigned int index = 0;

	memset(pBuffer, 0, 1024);

	// The Head Of CMD Message 0x55 0xaa 0x00 0x00
	unsigned char tempHead[] = { 0x55, 0xaa, 0x00, 0x00 };
	memcpy_s(pBuffer + index, 1024, tempHead, 4);
	index += 4;

	temp = 0x00000010;
	memcpy_s(pBuffer + index, 1024, &temp, 4);
	index += 4;
	
	GetDlgItemText(IDC_EDIT1, str);
	temp=_ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT2, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT3, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT4, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	temp = ((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCurSel();
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT5, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	temp = 1;//Start
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT6, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT7, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT8, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	temp = ((CComboBox*)GetDlgItem(IDC_COMBO2))->GetCurSel();
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT10, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT11, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	CAnCommPCI::GetInstance()->Send(pBuffer, 1024);

	free(pBuffer);

	SetTimer(RECEIVE_AS_FILE_TIMER, 1, NULL);
}


void CCommandToolDlg::OnEndBtnClicked()
{
	// TODO: Add your control notification handler code here
	CString str;
	unsigned char* pBuffer = (unsigned char*)malloc(1024);
	unsigned long temp = 0;
	unsigned long index = 0;

	memset(pBuffer, 0, 1024);

	// The Head Of CMD Message 0x55 0xaa 0x00 0x00
	unsigned char tempHead[] = { 0x55, 0xaa, 0x00, 0x00 };
	memcpy_s(pBuffer + index, 1024, tempHead, 4);
	index += 4;

	temp = 0x00000010;
	memcpy_s(pBuffer + index, 1024,&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT1, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT2, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT3, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT4, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	temp = ((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCurSel();
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT5, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	temp = 0; //Stop
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT6, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT7, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT8, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	temp = ((CComboBox*)GetDlgItem(IDC_COMBO2))->GetCurSel();
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT10, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	GetDlgItemText(IDC_EDIT11, str);
	temp = _ttoi(str);
	memcpy_s(pBuffer + index, 1024, (unsigned char*)&temp, 4);
	index += 4;

	CAnCommPCI::GetInstance()->Send(pBuffer, 1024);
	free(pBuffer);


	KillTimer(RECEIVE_AS_FILE_TIMER);
}




void CCommandToolDlg::OnReceiveBtnClicked()
{
	// TODO: Add your control notification handler code here
	unsigned char * pBuffer = (unsigned char *)malloc(0x40000);
	CAnCommPCI::GetInstance()->Receive((unsigned char *)pBuffer, 0x40000);
	CString str(pBuffer);
	free(pBuffer);
//	m_ReceiveTextBox.SetWindowText(str);

}


void CCommandToolDlg::OnSendAFileBtnClicked()
{
	// TODO: Add your control notification handler code here
	CFileDialog dialog(true);
	if (dialog.DoModal() == IDOK)
	{
		CString fileName = dialog.GetPathName();
		CFile file;
		if (!file.Open(fileName, CFile::modeRead))
		{
			MessageBox(_T("打开文件失败."));
			return;
		}
		unsigned long len = file.GetLength();
		if ((len%4)==0||len>8192)
		{
			MessageBox(_T("波表文件不正确."));			
		}
		else
		{
			unsigned char* pBuffer = (unsigned char *)malloc(len+4);
			unsigned char temp[] = { 0x55, 0xaa, 0x02, 0x00 };			
			memcpy_s(pBuffer, len + 4, temp, 4);

			unsigned long lenofdw = len / 4;
			memcpy_s(pBuffer + 4, len, &lenofdw, 4);

			file.Read(pBuffer+8, len);
			file.Flush();
			file.Close();
			CAnCommPCI::GetInstance()->Send(pBuffer, len);
			free(pBuffer);
		}
	}
}


void CCommandToolDlg::OnSetFilePathBtnClicked()
{
	// TODO: Add your control notification handler code here

	CFolderPickerDialog dialog;
	if (dialog.DoModal() == IDOK)
	{
		CAnCommPCI::GetInstance()->set_FileSavePath(dialog.GetFolderPath());
	}


}




void CCommandToolDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	switch (nIDEvent)
	{
		case RECEIVE_AS_FILE_TIMER:
			CAnCommPCI::GetInstance()->ReceiveAsFile();
			break;
		case GET_DEVICE_CURRENT_INFO_TIMER:
			DeviceStatus status;
			CAnCommPCI::GetInstance()->GetCurrentInfo(&status);
			TCHAR temp[20];
			_stprintf_s(temp,20, _T("%d"), status.voltage);
			SetDlgItemText(IDC_VOLTAGE, temp);
			_stprintf_s(temp, 20, _T("%d"), status.temperature);
			SetDlgItemText(IDC_TEMPERATURE, temp);
			_stprintf_s(temp, 20, _T("%d"), status.x);
			SetDlgItemText(IDC_DEVICE_X, temp);
			_stprintf_s(temp, 20, _T("%d"), status.y);
			SetDlgItemText(IDC_DEVICE_Y, temp);
			_stprintf_s(temp, 20, _T("%d"), status.z);
			SetDlgItemText(IDC_DEVICE_Z, temp);
			_stprintf_s(temp, 20, _T("%d"), status.comm);
			SetDlgItemText(IDC_COMM, temp);

			break;
		default:
			break;
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CCommandToolDlg::OnExitBtnClicked()
{
	// TODO: Add your message handler code here and/or call default


	OnCloseDeviceBtnClicked();
	exit(0);

}


void CCommandToolDlg::OnOpenFilePathBnClicked()
{
	// TODO: Add your message handler code here and/or call default

	//if (CAnCommPCI::GetInstance()->get_FileSavePath().IsEmpty())
	//{
	//	MessageBox(_T("未设置文件存储路径."));
	//}

	//ShellExecute(NULL, _T("open"),CAnCommPCI::GetInstance()->get_FileSavePath().GetBuffer(), NULL, NULL, SW_SHOWNORMAL);

}


void CCommandToolDlg::OnResetCommBtnClicked()
{
	// TODO: Add your control notification handler code here
	
	CAnCommPCI::GetInstance()->ResetComm();

}
