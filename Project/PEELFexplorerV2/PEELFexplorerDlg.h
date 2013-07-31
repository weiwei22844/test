// PEELFexplorerDlg.h : header file
//

#if !defined(AFX_PEELFEXPLORERDLG_H__84D18A8E_2DE8_4B9C_AE7B_AD67F152CF7F__INCLUDED_)
#define AFX_PEELFEXPLORERDLG_H__84D18A8E_2DE8_4B9C_AE7B_AD67F152CF7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CoffParser.h"
#include "ElfParser.h"
#include "PEParser.h"
/////////////////////////////////////////////////////////////////////////////
// CPEELFexplorerDlg dialog

class CPEELFexplorerDlg : public CDialog
{
// Construction
public:
	CPEELFexplorerDlg(CWnd* pParent = NULL);	// standard constructor

public:
    CString m_fileName;
    COFF_FILE_TYPE m_fileType;
    CCoffParser *m_pParser;

public:
    
// Dialog Data
	//{{AFX_DATA(CPEELFexplorerDlg)
	enum { IDD = IDD_PEELFEXPLORER_DIALOG };
	CTreeCtrl	m_FileInfoTree;
	CReportCtrl	m_fileRepList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPEELFexplorerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPEELFexplorerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAbout();
	afx_msg void OnFileExit();
	afx_msg void OnFileOpen();
	afx_msg void OnFileClose();
	afx_msg void OnSelchangedTreeFileinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PEELFEXPLORERDLG_H__84D18A8E_2DE8_4B9C_AE7B_AD67F152CF7F__INCLUDED_)
