
// CommandToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CommandTool.h"
#include "CommandToolDlg.h"
#include "afxdialogex.h"
#include "AnCommPCI.h"

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
	DDX_Control(pDX, IDC_EDIT1, m_SendTextBox);
	DDX_Control(pDX, IDC_EDIT2, m_ReceiveTextBox);
}

BEGIN_MESSAGE_MAP(CCommandToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CCommandToolDlg::OnOpenDeviceBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON2, &CCommandToolDlg::OnCloseDeviceBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON3, &CCommandToolDlg::OnSendBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON4, &CCommandToolDlg::OnReceiveBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON5, &CCommandToolDlg::OnSendAFileBtnClicked)
	ON_BN_CLICKED(IDC_BUTTON6, &CCommandToolDlg::OnReceiveAsFileBtnClicked)
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
		m_DeviceStatus.SetWindowText(_T("Opened"));
	else
	{
		MessageBox(_T("Open Device Failed."));
	}
	
}


void CCommandToolDlg::OnCloseDeviceBtnClicked()
{
	// TODO: Add your control notification handler code here
	CAnCommPCI::GetInstance()->CloseDevice();
	m_DeviceStatus.SetWindowText(_T("Closed"));

}


void CCommandToolDlg::OnSendBtnClicked()
{
	// TODO: Add your control notification handler code here
	CString str;
	m_SendTextBox.GetWindowText(str);
	CAnCommPCI::GetInstance()->Send((unsigned char *)str.GetBuffer(), str.GetLength());

}


void CCommandToolDlg::OnReceiveBtnClicked()
{
	// TODO: Add your control notification handler code here
	unsigned char * pBuffer = (unsigned char *)malloc(0x20000);
	CAnCommPCI::GetInstance()->Receive((unsigned char *)pBuffer, 0x20000);
	CString str(pBuffer);
	free(pBuffer);
	m_ReceiveTextBox.SetWindowText(str);

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
			MessageBox(_T("Open File Failed."));
			return;
		}
		unsigned long len = file.GetLength();
		unsigned char* pBuffer = (unsigned char *)malloc(len);
		file.Read(pBuffer, len); 
		file.Close();
		CAnCommPCI::GetInstance()->Send(pBuffer, len);
		free(pBuffer);
	}
}


void CCommandToolDlg::OnReceiveAsFileBtnClicked()
{
	// TODO: Add your control notification handler code here
	CFileDialog dialog(false);
	if (dialog.DoModal() == IDOK)
	{
		CString fileName = dialog.GetPathName();
		CFile file;
		if (!file.Open(fileName, CFile::modeWrite | CFile::modeCreate))
		{
			MessageBox(_T("Create File Failed."));
			return;
		}
		unsigned long len = 128 * 1024 * 1024;
		unsigned char* pBuffer = (unsigned char *)malloc(len);		
		len=CAnCommPCI::GetInstance()->Receive(pBuffer, len);
		file.Write(pBuffer, len);
		file.Flush();
		file.Close();
		free(pBuffer);
	}
}
