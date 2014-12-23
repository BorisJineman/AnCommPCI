
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
	afx_msg void OnStartBtnClicked();
	afx_msg void OnEndBtnClicked();
	afx_msg void OnReceiveBtnClicked();
	CStatic m_DeviceStatus;
	afx_msg void OnSendAFileBtnClicked();
	afx_msg void OnSetFilePathBtnClicked();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnExitBtnClicked();
	afx_msg void OnOpenFilePathBnClicked();
	afx_msg void OnResetCommBtnClicked();
};


#define RECEIVE_AS_FILE_TIMER 0x1000

#define GET_DEVICE_CURRENT_INFO_TIMER 0x1001