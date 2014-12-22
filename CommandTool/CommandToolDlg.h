
// CommandToolDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CCommandToolDlg dialog
class CCommandToolDlg : public CDialogEx
{
// Construction
public:
	CCommandToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_COMMANDTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpenDeviceBtnClicked();
	afx_msg void OnCloseDeviceBtnClicked();
	afx_msg void OnSendBtnClicked();
	afx_msg void OnReceiveBtnClicked();
	CStatic m_DeviceStatus;
	CEdit m_SendTextBox;
	CEdit m_ReceiveTextBox;
	afx_msg void OnSendAFileBtnClicked();
	afx_msg void OnReceiveAsFileBtnClicked();
};
