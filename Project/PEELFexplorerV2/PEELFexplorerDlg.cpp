// PEELFexplorerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PEELFexplorer.h"
#include "PEELFexplorerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPEELFexplorerDlg dialog

CPEELFexplorerDlg::CPEELFexplorerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPEELFexplorerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPEELFexplorerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_fileName = "";
    m_pParser = NULL;
}

void CPEELFexplorerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPEELFexplorerDlg)
	DDX_Control(pDX, IDC_TREE_FILEINFO, m_FileInfoTree);
	DDX_Control(pDX, IDC_LIST_FileReport, m_fileRepList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPEELFexplorerDlg, CDialog)
	//{{AFX_MSG_MAP(CPEELFexplorerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_ABOUT, OnAbout)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_FILEINFO, OnSelchangedTreeFileinfo)
	ON_WM_CLOSE()
	ON_WM_CANCELMODE()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPEELFexplorerDlg message handlers

BOOL CPEELFexplorerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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

void CPEELFexplorerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPEELFexplorerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPEELFexplorerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CPEELFexplorerDlg::OnAbout() 
{
	// TODO: Add your command handler code here
    CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CPEELFexplorerDlg::OnFileExit() 
{
	// TODO: Add your command handler code here
    OnFileClose();              // 退出程序是关闭打开文件
	OnCancel();
}

void CPEELFexplorerDlg::OnFileOpen() 
{
	// TODO: Add your command handler code here
    //CString filter = "pe File (*.exe)|*.exe|obj File (*.o)|*.o|All File (*.*)|*.*||";
    //CString filter = "All File (*.*)|*.*||";
    char filter[256];
    strcpy(filter, "ELF files (*.o, *.so)|*.o; *.so|PE files (*.exe, *.dll)|*.exe;*.dll|All files (*.*)|*.*|");
    
    CFileDialog fileDlg(TRUE, NULL, NULL, NULL, filter, this);
    // Specifies that the user can type only names of existing files in the File Name entry field. If this flag 
    // is specified and the user enters an invalid name, the dialog box procedure displays a warning in a message box
    fileDlg.m_ofn.Flags |= OFN_FILEMUSTEXIST;
    fileDlg.m_ofn.lpstrTitle = "Loading file...";
    if (fileDlg.DoModal() == IDOK) {
        // close the previous file
        OnFileClose();
        AfxGetApp()->BeginWaitCursor();
        m_fileName = fileDlg.GetPathName();	///Returns the full path of the selected file
        CString sPath;
        int nPos;
        GetModuleFileName(NULL, sPath.GetBufferSetLength(MAX_PATH+1), MAX_PATH);
        nPos = sPath.ReverseFind('\\');
        sPath = sPath.Right(sPath.GetLength() - nPos - 1);
        sPath.Format("%s - %s", sPath.GetBuffer(0), m_fileName.GetBuffer(0));
        AfxGetMainWnd()->SetWindowText(sPath);

        if(!m_pParser)
        {
            m_pParser = new CCoffParser;
        }
        if(m_pParser)
        {
            if(m_pParser->OpenFile(m_fileName.GetBuffer(0)))
            {
                m_fileType = m_pParser->GetFileType();
                delete m_pParser;
                m_pParser = NULL;
            }
            
            if(m_fileType == COFF_TYPE_ELF)
            {
                m_pParser = new CElfParser;
            }
            else if(m_fileType == COFF_TYPE_PE)
            {
                m_pParser = new CPEParser;
            }

            if(m_pParser && m_pParser->OpenFile(m_fileName.GetBuffer(0)))
            {
                m_pParser->ShowFileTree(&m_FileInfoTree);
            }
        }
	}
}

// added by ZWW to deal with ID_OK
BOOL CPEELFexplorerDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
    //if(wParam == IDOK || wParam == IDCANCEL) 
        //return TRUE;
	return CDialog::OnCommand(wParam, lParam);
}

BOOL CPEELFexplorerDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
    if(pMsg->message == WM_KEYDOWN) 
    { 
        UINT nKeyc=(UINT)(pMsg->wParam); 
        switch(nKeyc) 
        { 
        case VK_ESCAPE: 
            TRACE("Esc\n");
            pMsg->wParam = 0;
            break;
        case VK_RETURN: 
            TRACE("Enter\n");
            pMsg->wParam = 0; 
            break; 
        } 
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CPEELFexplorerDlg::OnFileClose() 
{
	// TODO: Add your command handler code here
    CString sPath;
    int nPos;
    GetModuleFileName(NULL, sPath.GetBufferSetLength(MAX_PATH+1), MAX_PATH);
    nPos = sPath.ReverseFind('\\');
    sPath = sPath.Right(sPath.GetLength() - nPos - 1);
    AfxGetMainWnd()->SetWindowText(sPath);
    m_FileInfoTree.DeleteAllItems();
    m_fileRepList.DeleteAllItems();
    m_fileRepList.DeleteAllColumns();
    if(m_pParser)
    {
        m_pParser->CloseFile();
        delete m_pParser;
        m_pParser = NULL;
    }
}

void CPEELFexplorerDlg::OnSelchangedTreeFileinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
    m_pParser->UpdateLstCtrl(&m_fileRepList, pNMTreeView);
	*pResult = 0;
}

void CPEELFexplorerDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	OnFileClose();              // 退出程序是关闭打开文件
	CDialog::OnClose();
}

void CPEELFexplorerDlg::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
    UINT count; 
    char filePath[200]; 
    count = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0); 
    if(count > 1) 
    {
        MessageBox("不支持对多文件打开");
    }else if(count == 1)
    {
        int pathLen = DragQueryFile(hDropInfo, 0, filePath, sizeof(filePath));
        CString tempfilename;
        tempfilename = filePath;
        
        if (tempfilename != m_fileName)                 //如果这次打开的文件与上次打开的文件是同一个
        {
            OnFileClose();
            m_fileName = tempfilename;
            AfxGetApp()->BeginWaitCursor();
            CString sPath;
            int nPos;
            GetModuleFileName(NULL, sPath.GetBufferSetLength(MAX_PATH+1), MAX_PATH);
            nPos = sPath.ReverseFind('\\');
            sPath = sPath.Right(sPath.GetLength() - nPos - 1);
            sPath.Format("%s - %s", sPath.GetBuffer(0), m_fileName.GetBuffer(0));
            AfxGetMainWnd()->SetWindowText(sPath);
            
            if(!m_pParser)
            {
                m_pParser = new CCoffParser;
            }
            if(m_pParser)
            {
                if(m_pParser->OpenFile(m_fileName.GetBuffer(0)))
                {
                    m_fileType = m_pParser->GetFileType();
                    delete m_pParser;
                    m_pParser = NULL;
                }
                
                if(m_fileType == COFF_TYPE_ELF)
                {
                    m_pParser = new CElfParser;
                }
                else if(m_fileType == COFF_TYPE_PE)
                {
                    m_pParser = new CPEParser;
                }
                
                if(m_pParser && m_pParser->OpenFile(m_fileName.GetBuffer(0)))
                {
                    m_pParser->ShowFileTree(&m_FileInfoTree);
                }
            }
        } 
    }
    DragFinish(hDropInfo);
	CDialog::OnDropFiles(hDropInfo);
}
