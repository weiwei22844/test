
// zmqDemoDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "sqlitedb.h"
#include "MyRichEdit.h"

#define ZMQPULL_PORT 5555
#define ZMQREQ_PORT 5556
#define ZMQPUSH_PORT 5557

// CzmqDemoDlg 对话框
class CzmqDemoDlg : public CDialog
{
// 构造
public:
	CzmqDemoDlg(CWnd* pParent = NULL);	// 标准构造函数

    LRESULT ShowTestInfo(WPARAM wp, LPARAM lp);

// 对话框数据
	enum { IDD = IDD_ZMQDEMO_DIALOG };

    BOOL m_bSerRun;
    BOOL m_bRespSerRun;
    void *m_pZmqCtx;
    SqliteDB m_database;

private:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT OnCallComing(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnStart();
    CMyRichEdit m_RichEditInfo;
    afx_msg void OnBnClickedBtnStartrep();
    afx_msg void OnBnClickedOk();
    CListCtrl m_ContactList;
    CListCtrl m_CalllogList;
    afx_msg void OnBnClickedBtnGetcontact();
    afx_msg void OnBnClickedBtnGetcalllog();
    UINT m_ContactOffset;
    UINT m_ContactSize;
    UINT m_CalllogOffset;
    UINT m_CalllogSize;
    afx_msg void OnBnClickedBtnStop();
    afx_msg void OnDestroy();

    int getCalllog(int offset, int size);
    int getContacts(int offset, int size);
    int addContact(const char* pName, const char* pSipAddr, int subscribe, int SubscribePolicy, int Status, const char* pImage);
    int deleteContact(int index);
    int deleteCalllog(int index);
    afx_msg void OnClose();
    afx_msg void OnBnClickedBtnCall();
    afx_msg void OnBnClickedBtnTerminate();

    afx_msg void OnBnClickedBtnChgstatus();
    afx_msg void OnBnClickedBtnLeft();
    afx_msg void OnBnClickedBtnRight();
    afx_msg void OnBnClickedBtnUp();
    afx_msg void OnBnClickedBtnDown();
    afx_msg void OnBnClickedBtnenter();
    afx_msg void OnBnClickedBtnOnhook();
    afx_msg void OnBnClickedBtnOffhook();
    afx_msg void OnBnClickedBtnEsc();
    afx_msg void OnBnClickedBtnMenu();
};
