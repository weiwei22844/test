
// zmqDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "zmqDemo.h"
#include "zmqDemoDlg.h"
#include "zmq.h"
#include "zmq_utils.h"

#include "json/json.h"
#include <algorithm> // sort
#include "sqlitedb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_MSG_CALLCOMING (WM_USER+100)

CzmqDemoDlg* pMainDlg = NULL;
void* pPushSocket = NULL;
char g_cInfoBuf[500];
char g_cSendBuff[10240]={0};

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CzmqDemoDlg 对话框




CzmqDemoDlg::CzmqDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CzmqDemoDlg::IDD, pParent)
    , m_ContactOffset(0)
    , m_ContactSize(0)
    , m_CalllogOffset(0)
    , m_CalllogSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pZmqCtx = NULL;
}

void CzmqDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RICHEDIT_INFO, m_RichEditInfo);
    DDX_Control(pDX, IDC_LIST_CONTACT, m_ContactList);
    DDX_Control(pDX, IDC_LIST_CALLLOG, m_CalllogList);
    DDX_Text(pDX, IDC_EDIT_CONTOFFSET, m_ContactOffset);
    DDX_Text(pDX, IDC_EDIT_CONTSIZE, m_ContactSize);
    DDX_Text(pDX, IDC_EDIT_CALLOFFSET, m_CalllogOffset);
    DDX_Text(pDX, IDC_EDIT_CALLSIZE, m_CalllogSize);
}

BEGIN_MESSAGE_MAP(CzmqDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTN_START, &CzmqDemoDlg::OnBnClickedBtnStart)
    ON_BN_CLICKED(IDC_BTN_STARTREP, &CzmqDemoDlg::OnBnClickedBtnStartrep)
    ON_BN_CLICKED(IDOK, &CzmqDemoDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BTN_GETCONTACT, &CzmqDemoDlg::OnBnClickedBtnGetcontact)
    ON_BN_CLICKED(IDC_BTN_GETCALLLOG, &CzmqDemoDlg::OnBnClickedBtnGetcalllog)
    ON_BN_CLICKED(IDC_BTN_STOP, &CzmqDemoDlg::OnBnClickedBtnStop)
    ON_WM_DESTROY()
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BTN_CALL, &CzmqDemoDlg::OnBnClickedBtnCall)
    ON_BN_CLICKED(IDC_BTN_TERMINATE, &CzmqDemoDlg::OnBnClickedBtnTerminate)

    ON_MESSAGE(WM_MSG_CALLCOMING, &CzmqDemoDlg::OnCallComing)
END_MESSAGE_MAP()


// CzmqDemoDlg 消息处理程序

BOOL CzmqDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    m_bSerRun = FALSE;
    m_bRespSerRun = FALSE;
    char title[100];
    int major, minor, patch;
    zmq_version (&major, &minor, &patch);
    sprintf (title, "ZMQ %d.%d.%d\n", major, minor, patch);
    SetWindowText(title);

    pMainDlg = this;

    int rt;
    rt = m_database.InitDataBase("Bell.db3");
    if(rt < 0){
        ShowTestInfo(0, (LPARAM)"初始化数据库连接失败\n");
    }else{
        ShowTestInfo(0, (LPARAM)"初始化数据库连接成功\n");
    }

    m_ContactList.InsertColumn(0, _T("Id"), LVCFMT_LEFT, 30);
    m_ContactList.InsertColumn(1, _T("UserName"), LVCFMT_LEFT, 50);
    m_ContactList.InsertColumn(2, _T("SipAddress"), LVCFMT_LEFT, 150);
    m_ContactList.InsertColumn(3, _T("Subscribe"), LVCFMT_LEFT, 50);
    m_ContactList.InsertColumn(4, _T("SubscribePolicy"), LVCFMT_LEFT, 50);
    m_ContactList.InsertColumn(5, _T("Status"), LVCFMT_LEFT, 50);
    m_ContactList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

    m_CalllogList.InsertColumn(0, _T("Id"), LVCFMT_LEFT, 30);
    m_CalllogList.InsertColumn(1, _T("Callin"), LVCFMT_LEFT, 80);
    m_CalllogList.InsertColumn(2, _T("Disconnect"), LVCFMT_LEFT, 80);
    m_CalllogList.InsertColumn(3, _T("Caller"), LVCFMT_LEFT, 150);
    m_CalllogList.InsertColumn(4, _T("Answer"), LVCFMT_LEFT, 150);
    m_CalllogList.InsertColumn(5, _T("StartDate"), LVCFMT_LEFT, 100);
    m_CalllogList.InsertColumn(6, _T("Duration"), LVCFMT_LEFT, 50);
    m_CalllogList.InsertColumn(7, _T("Quality"), LVCFMT_LEFT, 50);
    m_CalllogList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

    CString str;
    str.Format("联系人总数：%d", m_database.getContactCount());
    ((CStatic*)GetDlgItem(IDC_STATIC_CONTACT))->SetWindowText(str);

    str.Format("通话记录总数：%d", m_database.getCallLogCount());
    ((CStatic*)GetDlgItem(IDC_STATIC_CALLLOG))->SetWindowText(str);

    m_pZmqCtx = zmq_init (1);
    if (!m_pZmqCtx) {
        TRACE ("error in zmq_init: %s\n", zmq_strerror (errno));
        ShowTestInfo(0, (LPARAM)"zmq_init failed!\r\n");
        return FALSE;
    }else{
        ShowTestInfo(0, (LPARAM)"zmq_init OK!\r\n");
    }

    pPushSocket = zmq_socket (pMainDlg->m_pZmqCtx, ZMQ_PUSH);
    if (!pPushSocket) {
        TRACE ("error in zmq_socket push: %s\n", zmq_strerror (errno));
        ShowTestInfo(0, (LPARAM)"zmq_socket push create failed！\r\n");
    }else{
        ShowTestInfo(0, (LPARAM)"zmq_socket push create OK！\r\n");
    }

    // by ZWW 20130715
    char temp[100];
#if 1           // for Ubuntu
    sprintf(temp, "tcp://192.168.1.146:%d", ZMQPULL_PORT);
#else
    sprintf(temp, "tcp://192.168.1.12:%d", ZMQPULL_PORT);
#endif
    //if (zmq_connect (pPushSocket, "tcp://192.168.1.222:5555") != 0) {
    if (zmq_connect (pPushSocket, temp) != 0) {
        printf ("error in zmq_connect push: %s\n", zmq_strerror (errno));
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"zmq_connect push tcp://192.168.1.12:5555 OK！\r\n");
    int nTimeOut = 1000;
    if(zmq_setsockopt (pPushSocket, ZMQ_LINGER, &nTimeOut, sizeof(nTimeOut)) != 0){
        printf ("error in zmq_setsockopt ZMQ_LINGER: %s\n", zmq_strerror (errno));
    }

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CzmqDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CzmqDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CzmqDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void ShowMessage(char* pMsg)
{
    if(pMainDlg){
        pMainDlg->ShowTestInfo(0, (LPARAM)pMsg);
    }
}

int sendResult(char *pData, int len)
{
    int rt;
    zmq_msg_t msg;
    rt = zmq_msg_init_size (&msg, len);
    if (rt != 0) {
        sprintf (g_cInfoBuf, "error in zmq_msg_init_size of sendResult: %s\n", zmq_strerror (errno));
        ShowMessage(g_cInfoBuf);
        return -1;
    }

    memcpy (zmq_msg_data (&msg), pData, len);

    if(pPushSocket){
        ShowMessage("will send msg:");
        ShowMessage(pData);
        rt = zmq_sendmsg (pPushSocket, &msg, 0);
        if (rt < 0) {
            sprintf (g_cInfoBuf, "error in zmq_sendmsg of sendResult: %s\n", zmq_strerror (errno));
            ShowMessage(g_cInfoBuf);
            return -1;
        }
    }else{
        return -1;
    }
    
    zmq_msg_close (&msg);
    return len;
}

static void printValueTree( Json::Value &value, const std::string &path = "." )
{
    char tmp[100];
    switch ( value.type() )
    {
    case Json::nullValue:
        TRACE( "%s=null\n", path.c_str() );
        sprintf(tmp, "%s=null\n", path.c_str() );
        ShowMessage(tmp);
        break;
    case Json::intValue:
        TRACE( "%s=%d\n", path.c_str(), value.asInt());
        sprintf(tmp, path.c_str(), value.asInt() );
        ShowMessage(tmp);
        break;
    case Json::uintValue:
        TRACE( "%s=%u\n", path.c_str(), value.asUInt() );
        sprintf(tmp, path.c_str(), value.asUInt() );
        ShowMessage(tmp);
        break;
    case Json::realValue:
        TRACE( "%s=%.16g\n", path.c_str(), value.asDouble() );
        sprintf(tmp, path.c_str(), value.asDouble() );
        ShowMessage(tmp);
        break;
    case Json::stringValue:
        TRACE( "%s=\"%s\"\n", path.c_str(), value.asString().c_str() );
        sprintf(tmp, path.c_str(), value.asString().c_str() );
        ShowMessage(tmp);
        break;
    case Json::booleanValue:
        TRACE( "%s=%s\n", path.c_str(), value.asBool() ? "true" : "false" );
        sprintf(tmp, path.c_str(), value.asBool() ? "true" : "false" );
        ShowMessage(tmp);
        break;
    case Json::arrayValue:
        {
            TRACE( "%s=[]\n", path.c_str() );
            sprintf(tmp, path.c_str() );
            ShowMessage(tmp);
            int size = value.size();
            for ( int index =0; index < size; ++index )
            {
                static char buffer[16];
                sprintf( buffer, "[%d]", index );
                printValueTree( value[index], path + buffer );
            }
        }
        break;
    case Json::objectValue:
        {
            TRACE( "%s={}\n", path.c_str() );
            sprintf(tmp, path.c_str() );
            ShowMessage(tmp);
            Json::Value::Members members( value.getMemberNames() );
            std::sort( members.begin(), members.end() );
            std::string suffix = *(path.end()-1) == '.' ? "" : ".";
            for ( Json::Value::Members::iterator it = members.begin(); 
                it != members.end(); ++it )
            {
                const std::string &name = *it;
                printValueTree( value[name], path + suffix + name );
            }
        }
        break;
    default:
        break;
    }
}

void processRequest(Json::Value root)
{
    const char *pMethod = root["method"].asCString();
    int id = root["id"].asInt();
    if(strcmp(pMethod, "user_login") == 0){
        if(!root["params"].isNull())
        {
            Json::Value params = root["params"];
            const char *pName = NULL, *pPwd = NULL;
            if(params.type() == Json::objectValue){
                pName = params["username"].asCString();
                pPwd = params["passwd"].asCString();
                if(strcmp(pName, "1") == 0 && strcmp(pPwd, "1") == 0){
                    sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"login success\", \"id\": %d}", id);
                }else{
                    sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"login failed\", \"id\": %d}", id);
                }
            }else{
                sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"login success\", \"id\": %d}", id);
            }
        }else{
            sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"error login request format\", \"id\": %d}", id);
        }
        sendResult(g_cSendBuff, strlen(g_cSendBuff));
    }else if(strcmp(pMethod, "user_logout") == 0){
        sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"logout success\", \"id\": %d}", id);
        sendResult(g_cSendBuff, strlen(g_cSendBuff));
    }else if(strcmp(pMethod, "invite_remote") == 0){
        if(!PostMessage(pMainDlg->m_hWnd, WM_MSG_CALLCOMING,(WPARAM)id,0))
        {
            ShowMessage("PostMessage failed");
        }
    }else if(strcmp(pMethod, "get_calllog_num") == 0){
        sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": %d, \"id\": %d}", pMainDlg->m_database.getCallLogCount(), id);
        sendResult(g_cSendBuff, strlen(g_cSendBuff));
    }else if(strcmp(pMethod, "get_calllog") == 0){
        int nOffset, nSize;
        Json::Value params = root["params"];
        int nRecordCnt;
        if(params.type() == Json::arrayValue){
            int i = 0;
            if(params.size() == 2){
                nOffset = params[i].asInt();
                i++;
                nSize = params[i].asInt();
                memset(g_cSendBuff, 0, sizeof(g_cSendBuff));
                sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"id\": %d, \"result\":{\"calllog\":[", id);
                nRecordCnt = pMainDlg->getCalllog(nOffset, nSize);
                if(nRecordCnt > 0)
                    g_cSendBuff[strlen(g_cSendBuff) - 1] = 0;       // delete the "," at the end
                strcat(g_cSendBuff, "]}}");
                sendResult(g_cSendBuff, strlen(g_cSendBuff));
            }else{
                ShowMessage("get calllog wrong params size\r\n");
            }
        }else if(params.type() ==  Json::objectValue){
            if(!params["offset"].isNull() && !params["limit"].isNull()){
                nOffset = params["offset"].asInt();
                nSize = params["limit"].asInt();
                memset(g_cSendBuff, 0, sizeof(g_cSendBuff));
                sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"id\": %d, \"result\":{\"calllog\":[", id);
                nRecordCnt = pMainDlg->getCalllog(nOffset, nSize);
                if(nRecordCnt > 0)
                    g_cSendBuff[strlen(g_cSendBuff) - 1] = 0;       // delete the "," at the end
                strcat(g_cSendBuff, "]}}");
                sendResult(g_cSendBuff, strlen(g_cSendBuff));
            }else{
                ShowMessage("get calllog wrong params\r\n");
            }
        }else{
            ShowMessage("get calllog wrong request format\r\n");
        }
    }else if(strcmp(pMethod, "get_friend") == 0){
        int nOffset, nSize;
        int nRecordCnt;
        Json::Value params = root["params"];
        if(params.type() == Json::arrayValue){
            int i = 0;
            if(params.size() == 2){
                nOffset = params[i].asInt();
                i++;
                nSize = params[i].asInt();
                memset(g_cSendBuff, 0, sizeof(g_cSendBuff));
                sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"id\": %d, \"result\":{\"friend\":[", id);
                nRecordCnt = pMainDlg->getContacts(nOffset, nSize);
                if(nRecordCnt > 0)
                    g_cSendBuff[strlen(g_cSendBuff) - 1] = 0;       // delete the "," at the end
                strcat(g_cSendBuff, "]}}");
                sendResult(g_cSendBuff, strlen(g_cSendBuff));
            }else{
                ShowMessage("get contact wrong params size\r\n");
            }
        } else if(params.type() ==  Json::objectValue){
            if(!params["offset"].isNull() && !params["limit"].isNull()){
                nOffset = params["offset"].asInt();
                nSize = params["limit"].asInt();
                memset(g_cSendBuff, 0, sizeof(g_cSendBuff));
                sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"id\": %d, \"result\":{\"friend\":[", id);
                nRecordCnt = pMainDlg->getContacts(nOffset, nSize);
                if(nRecordCnt > 0)
                    g_cSendBuff[strlen(g_cSendBuff) - 1] = 0;       // delete the "," at the end
                strcat(g_cSendBuff, "]}}");
                sendResult(g_cSendBuff, strlen(g_cSendBuff));
            }else{
                ShowMessage("get contact wrong params\r\n");
            }
        }else{
            ShowMessage("get contact wrong request format\r\n");
        }
    }else if(strcmp(pMethod, "del_friend") == 0){
        Json::Value params = root["params"];
        if(params.type() == Json::objectValue){
            int index = params["index"].asInt();
            if(pMainDlg->deleteContact(index) > 0){
                sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"del_data success\", \"id\": %d}", id);
            }else{
                sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"del_data failed\", \"id\": %d}", id);
            }
            sendResult(g_cSendBuff, strlen(g_cSendBuff));
        }
    }else if(strcmp(pMethod, "add_friend") == 0){
        Json::Value params = root["params"];
        sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"add_friend success\", \"id\": %d}", id);
        sendResult(g_cSendBuff, strlen(g_cSendBuff));
    }else if(strcmp(pMethod, "edit_friend") == 0){
        sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"edit_friend success\", \"id\": %d}", id);
        sendResult(g_cSendBuff, strlen(g_cSendBuff));
    }else if(strcmp(pMethod, "hangup_current_call") == 0){
        sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"LinphoneCallEnd\", \"id\": %d}", id);
        sendResult(g_cSendBuff, strlen(g_cSendBuff));
    }else if(strcmp(pMethod, "get_cur_sip_account") == 0){
        sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"sip:22@192.168.1.112\", \"id\": %d}", id);
        sendResult(g_cSendBuff, strlen(g_cSendBuff));
    }else if(strcmp(pMethod, "change_cur_sip_account") == 0){
        sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"change_cur_sip_account success\", \"id\": %d}", id);
        sendResult(g_cSendBuff, strlen(g_cSendBuff));
    }else if(strcmp(pMethod, "get_usercase") == 0){
        sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"camera\", \"id\": %d}", id);
        sendResult(g_cSendBuff, strlen(g_cSendBuff));
    }
}

void processNotification(Json::Value root)
{
    const char *pMethod = root["method"].asCString();
    if(strcmp(pMethod, "clear_calllog") == 0){
        ShowMessage("Will delete all the call log!\r\n");
    }
}

void processReply(Json::Value root)
{

}

void processRecvData(char* pData, int nSize)
{
#if 1
    std::string text(pData);
#else
    FILE *file = fopen( "C:\\mywork\\my\\jsoncpp-src-0.5.0\\build\\vs71\\debug\\jsontest\\jsonTest.json", "rb" );
    if ( !file )
        return;
    fseek( file, 0, SEEK_END );
    long size = ftell( file );
    fseek( file, 0, SEEK_SET );
    std::string text;
    char *buffer = new char[size+1];
    buffer[size] = 0;
    if ( fread( buffer, 1, size, file ) == (unsigned long)size )
        text = buffer;
    fclose( file );
    delete[] buffer;
#endif

    Json::Features features;
    Json::Reader reader( features );
    Json::Value root;
    if (reader.parse(text, root))
    {
#if 0
        printValueTree(root);
#else
        if(!root["method"].isNull()){
            if(!root["id"].isNull()){               // request
                processRequest(root);
            }else{                                  // notification
                processNotification(root);
            }
        }else if(!root["result"].isNull()){         // reply
            processReply(root);
        }else{                                      // exception
            sendResult("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32700, \"message\": \"Parse error\"}, \"id\": null}", 
                strlen("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32700, \"message\": \"Parse error\"}, \"id\": null}"));
        }
#endif
        /*value.Members
        int recordCount = value["recordCount"].asInt();
        if(recordCount > 0)                             // 联系人总人数
        {
            const Json::Value arrayObj = value["rows"];
            int index = m_ListContact.GetItemCount();
            int nSize = arrayObj.size();
            if(nSize < 10)
                bComplet = TRUE;
            for (int i=0; i<nSize; i++)
            {
                std::string out = arrayObj[i]["passenger_name"].asString();
                m_ListContact.InsertItem(index, out.c_str());
                out = arrayObj[i]["passenger_id_no"].asString();
                m_ListContact.SetItemText(index, 1, out.c_str());
                index++;
            }
        }else{
            bComplet = TRUE;
        }*/
    }else{
        sendResult("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32600, \"message\": \"Invalid JSON-RPC.\"}, \"id\": null}", 
            strlen("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32600, \"message\": \"Invalid JSON-RPC.\"}, \"id\": null}"));
    }
}

UINT ServerFunc(void *param)
{
    CzmqDemoDlg *pMainDlg = NULL;
    int rc;
    pMainDlg = (CzmqDemoDlg *)param;

    void *ctx;
    void *s;
    zmq_msg_t msg;
    void *watch;
    unsigned long elapsed;
    char recv[1024]={0};

    /*if(!pMainDlg->m_pZmqCtx){
        pMainDlg->m_bSerRun = FALSE;
        return 0;
    }*/

    ctx = zmq_init (1);
    if (!ctx) {
        TRACE ("error in zmq_init: %s\n", zmq_strerror (errno));
        return 0;
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"zmq_init OK！\r\n");

    s = zmq_socket (ctx, ZMQ_PULL);
    if (!s) {
        TRACE ("error in zmq_socket: %s\n", zmq_strerror (errno));
        pMainDlg->m_bSerRun = FALSE;
        return 0;
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"zmq_socket ZMQ_PULL OK！\r\n");

    //  Add your socket options here.
    //  For example ZMQ_RATE, ZMQ_RECOVERY_IVL and ZMQ_MCAST_LOOP for PGM.

    sprintf(recv, "tcp://192.168.1.12:%d", ZMQPUSH_PORT);
    rc = zmq_bind (s, recv);
    if (rc != 0) {
        TRACE ("error in zmq_bind: %s\n", zmq_strerror (errno));
        pMainDlg->m_bSerRun = FALSE;
        return 0;
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"zmq_bind tcp://192.168.1.12 OK！\r\n");

    rc = zmq_msg_init (&msg);
    if (rc != 0) {
        TRACE ("error in zmq_msg_init: %s\n", zmq_strerror (errno));
        pMainDlg->m_bSerRun = FALSE;
        return 0;
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"zmq_msg_init OK！\r\n");

    int nTimeOut = 1000;
    rc = zmq_setsockopt (s, ZMQ_RCVTIMEO, &nTimeOut, sizeof(nTimeOut));
    if (rc != 0) {
        printf ("error in zmq_setsockopt ZMQ_RCVTIMEO: %s\n", zmq_strerror (errno));
    }
    nTimeOut = 1000;
    rc = zmq_setsockopt (s, ZMQ_LINGER, &nTimeOut, sizeof(nTimeOut));
    if(rc != 0) {
        printf ("error in zmq_setsockopt ZMQ_LINGER: %s\n", zmq_strerror (errno));
    }

    watch = zmq_stopwatch_start ();
    while( pMainDlg->m_bSerRun ) {
        rc = zmq_recvmsg (s, &msg, 0);
        if (rc < 0) {
            printf ("error in zmq_recvmsg: %s\n", zmq_strerror (errno));
            continue;
        }
        
        memset(recv, 0, sizeof(recv));
        memcpy(recv, zmq_msg_data (&msg), zmq_msg_size (&msg));
        processRecvData(recv, zmq_msg_size (&msg));
        pMainDlg->ShowTestInfo(0, (LPARAM)recv);
    }
    elapsed = zmq_stopwatch_stop (watch);
    if (elapsed == 0)
        elapsed = 1;

    rc = zmq_msg_close (&msg);
    if (rc != 0) {
        TRACE ("error in zmq_msg_close: %s\n", zmq_strerror (errno));
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"pull message close......\r\n");

    rc = zmq_close (s);
    if (rc != 0) {
        TRACE ("error in zmq_close: %s\n", zmq_strerror (errno));
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"pull socket close......\r\n");

    rc = zmq_term (ctx);
    if (rc != 0) {
        TRACE ("error in zmq_term: %s\n", zmq_strerror (errno));
        return 0;
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"pull server stop OK!\r\n");

    return 1;
}

void CzmqDemoDlg::OnBnClickedBtnStart()
{
    // TODO: 在此添加控件通知处理程序代码
    if(!m_bSerRun){
        m_bSerRun = TRUE;
        if(!AfxBeginThread(ServerFunc, this, NULL))
        {
            int err = GetLastError();
            TRACE("create ServerFunc thread failed last error: %d", err);
            m_bSerRun = FALSE;
        }
    }
}

LRESULT CzmqDemoDlg::ShowTestInfo(WPARAM wp, LPARAM lp)
{
	int nMsgInfo = (int)wp;

	char *info = (char*)lp;
	m_RichEditInfo.SetBackgroundColor(TRUE, RGB(255, 255, 255));
	CString str(info);

	int cntStr = m_RichEditInfo.GetWindowTextLength();  
	m_RichEditInfo.SetSel(cntStr, cntStr);  

	SYSTEMTIME st;
	CString strDate;
    char chTimeInfo[50] = {0};
	GetLocalTime(&st);
    sprintf_s(chTimeInfo, "[%02d-%02d %02d:%02d:%02d] ", st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	strDate.Format(_T("\r\n[%02d-%02d %02d:%02d:%02d] "), st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	str = strDate + str;

	m_RichEditInfo.ReplaceSel(str);
	m_RichEditInfo.HideSelection(FALSE, FALSE);

	return 1;
}

UINT RespServerFunc(void *param)
{
    CzmqDemoDlg *pMainDlg = NULL;
    pMainDlg = (CzmqDemoDlg *)param;

    int roundtrip_count;
    size_t message_size;
    void *ctx;
    void *s;
    int rc;
    zmq_msg_t msg;

    roundtrip_count = 11;
    message_size = 100;
    ctx = zmq_init (1);
    if (!ctx) {
        TRACE ("error in zmq_init: %s\n", zmq_strerror (errno));
        return 0;
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"zmq_init OK！\r\n");

    s = zmq_socket (ctx, ZMQ_REP);
    if (!s) {
        TRACE ("error in zmq_socket: %s\n", zmq_strerror (errno));
        return 0;
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"zmq_socket ZMQ_REP OK！\r\n");

    //  Add your socket options here.
    //  For example ZMQ_RATE, ZMQ_RECOVERY_IVL and ZMQ_MCAST_LOOP for PGM.

    rc = zmq_bind (s, "tcp://192.168.1.12:5556");
    if (rc != 0) {
        TRACE ("error in zmq_bind: %s\n", zmq_strerror (errno));
        return 0;
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"zmq_bind tcp://192.168.1.12:5556 OK！\r\n");

    rc = zmq_msg_init (&msg);
    if (rc != 0) {
        TRACE ("error in zmq_msg_init: %s\n", zmq_strerror (errno));
        return 0;
    }
    pMainDlg->ShowTestInfo(0, (LPARAM)"zmq_msg_init OK！\r\n");

    for (int i = 0; i != roundtrip_count - 1 && pMainDlg->m_bRespSerRun; i++) {
        rc = zmq_recvmsg (s, &msg, 0);
        if (rc < 0) {
            TRACE ("error in zmq_recvmsg: %s\n", zmq_strerror (errno));
            break;
        }
        /*if (zmq_msg_size (&msg) != message_size) {
            printf ("message of incorrect size received\n");
            break;
        }*/
        pMainDlg->ShowTestInfo(0, (LPARAM)zmq_msg_data (&msg));

        Sleep(5000);

        sprintf ((char*)(zmq_msg_data (&msg)), "return by server %d by ZWW", i);
        rc = zmq_sendmsg(s, &msg, 0);
        if(rc < 0){
            TRACE("error in zmq_sendmsg: %s\n", zmq_strerror (errno));
        }
    }

    rc = zmq_msg_close (&msg);
    if (rc != 0) {
        TRACE ("error in zmq_msg_close: %s\n", zmq_strerror (errno));
        return 0;
    }

    zmq_sleep(1);

    rc = zmq_close (s);
    if (rc != 0) {
        TRACE ("error in zmq_close: %s\n", zmq_strerror (errno));
        return 0;
    }

    rc = zmq_term (ctx);
    if (rc != 0) {
        TRACE ("error in zmq_term: %s\n", zmq_strerror (errno));
        return 0;
    }

    return 1;
}

void CzmqDemoDlg::OnBnClickedBtnStartrep()
{
    // TODO: 在此添加控件通知处理程序代码
    if(!m_bRespSerRun){
        m_bRespSerRun = TRUE;
        if(!AfxBeginThread(RespServerFunc, this, NULL))
        {
            int err = GetLastError();
            TRACE("create RespServerFunc thread failed last error: %d", err);
            m_bRespSerRun = FALSE;
        }
    }
}

void CzmqDemoDlg::OnBnClickedOk()
{
    // TODO: 在此添加控件通知处理程序代码
    //OnOK();
    /*int rt;
    rt = m_database.InitDataBase("Bell.db3");
    if(rt < 0){
        MessageBox("初始化数据库连接失败");
    }else{
        MessageBox("初始化数据库连接成功");
    }*/

    /*std::string text("{\"params\":[1 ,2]}");
    Json::Features features;
    Json::Reader reader( features );
    Json::Value root;
    int tmp;
    if (reader.parse(text, root)){
        Json::Value params = root["params"];
        if(params.type() == Json::arrayValue){
            for(int i = 0; i < params.size(); i++){
                tmp = params[i].asInt();
            }
        }
    }*/

    /*char temp[200];
    std::string text("{\"result\":[{\"id\":1,\"direction\":\"to\",\"from\":\"(null)\",\"videoEnabled\":1},{\"id\": 2,\"direction\":\"from\",\"from\":\"sip:101@192.168.1.112\",\"videoEnabled\":0}]}");
    Json::Features features;
    Json::Reader reader( features );
    Json::Value root;
    int tmp;
    if (reader.parse(text, root)){
        Json::Value params = root["params"];
        if(params.type() == Json::arrayValue){
            for(int i = 0; i < params.size(); i++){
                tmp = params[i].asInt();
            }
        }
    }else{
        sprintf(temp, "Failed to parse: \n%s\n", reader.getFormatedErrorMessages().c_str() );
        OutputDebugString(temp);
    }*/

    if(m_database.delContact(33) < 0){
        OutputDebugString(m_database.getlastErrorInfo());
    }
}

static int getContactCb(void *pUserData, int nColumn, char **columnValue, char **columnName){
    int index = pMainDlg->m_ContactList.GetItemCount();
    pMainDlg->m_ContactList.InsertItem(index, columnValue[0]);
    for(int i=1; i < nColumn; i++)
    {
        pMainDlg->m_ContactList.SetItemText(index, i, columnValue[i]);
    }
    return 0;
}

void CzmqDemoDlg::OnBnClickedBtnGetcontact()
{
    // TODO: 在此添加控件通知处理程序代码
    UpdateData();
    m_ContactList.DeleteAllItems();
    m_database.getContact(m_ContactOffset, m_ContactSize, getContactCb, this);
}

static int getCallLogCb(void *pUserData, int nColumn, char **columnValue, char **columnName){
    int index = pMainDlg->m_CalllogList.GetItemCount();
    pMainDlg->m_CalllogList.InsertItem(index, columnValue[0]);
    for(int i=1; i < nColumn; i++)
    {
        pMainDlg->m_CalllogList.SetItemText(index, i, columnValue[i]);
    }
    return 0;
}

static int getCallLogCbResp(void *pUserData, int nColumn, char **columnValue, char **columnName){
    char temp[200] = {0};
    sprintf(temp, "{\"%s\": %d,", columnName[0], atoi(columnValue[0]));
    strcat(g_cSendBuff, temp);
    for(int i=1; i < nColumn; i++)
    {
        if(i == nColumn -1)
            sprintf(temp, "\"%s\":%d},", columnName[i], atoi(columnValue[i]));
        else if(i == nColumn - 2)
            sprintf(temp, "\"%s\":%d,", columnName[i], atoi(columnValue[i]));
        else
            sprintf(temp, "\"%s\":\"%s\",", columnName[i], columnValue[i]);
        strcat(g_cSendBuff, temp);
    }
    return 0;
}

static int getContactCbResp(void *pUserData, int nColumn, char **columnValue, char **columnName){
    char temp[200] = {0};
    sprintf(temp, "{\"%s\": %d,", columnName[0], atoi(columnValue[0]));
    strcat(g_cSendBuff, temp);
    for(int i=1; i < nColumn; i++)
    {
        if(i == nColumn -1)
            sprintf(temp, "\"%s\":%d},", columnName[i], atoi(columnValue[i]));
        else if(i == nColumn - 2)
            sprintf(temp, "\"%s\":%d,", columnName[i], atoi(columnValue[i]));
        else
            sprintf(temp, "\"%s\":\"%s\",", columnName[i], columnValue[i]);
        strcat(g_cSendBuff, temp);
    }
    return 0;
}

void CzmqDemoDlg::OnBnClickedBtnGetcalllog()
{
    // TODO: 在此添加控件通知处理程序代码
    UpdateData();
    m_CalllogList.DeleteAllItems();
    m_database.getCalllog(m_CalllogOffset, m_CalllogSize, getCallLogCb, this);
}

int CzmqDemoDlg::getCalllog(int offset, int size)
{
    return m_database.getCalllog(offset, size, getCallLogCbResp, this);
}

int CzmqDemoDlg::getContacts(int offset, int size)
{
    return m_database.getContact(offset, size, getContactCbResp, this);
}

int CzmqDemoDlg::deleteContact(int index)
{
    return m_database.delContact(index);
}

void CzmqDemoDlg::OnBnClickedBtnStop()
{
    // TODO: 在此添加控件通知处理程序代码
    if(!m_bSerRun){
        ShowTestInfo(0, (LPARAM)"Pull server not run\n");
    }else{
        m_bSerRun = FALSE;
        ShowTestInfo(0, (LPARAM)"Pull server will stop, please wait\n");
    }
}

void CzmqDemoDlg::OnDestroy()
{
    CDialog::OnDestroy();

    // TODO: 在此处添加消息处理程序代码
    if(pPushSocket){
        int nTimeOut = 1000;
        if (zmq_setsockopt (pPushSocket, ZMQ_SNDTIMEO, &nTimeOut, sizeof(nTimeOut)) != 0) {
            TRACE ("error in zmq_setsockopt: %s\n", zmq_strerror (errno));
        }
        zmq_close(pPushSocket);
    }
    if(m_pZmqCtx){
        if (zmq_term (m_pZmqCtx) != 0) {
            TRACE ("error in zmq_term: %s\n", zmq_strerror (errno));
        }
    }
}

void CzmqDemoDlg::OnClose()
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    OnBnClickedBtnStop();

    CDialog::OnClose();
}

void CzmqDemoDlg::OnBnClickedBtnCall()
{
    // TODO: 在此添加控件通知处理程序代码
    sendResult("{\"jsonrpc\": \"2.0\", \"method\": \"remote_call\",  \"params\": \"123\", \"id\": 1}", 
        strlen("{\"jsonrpc\": \"2.0\", \"method\": \"remote_call\",  \"params\": \"123\", \"id\": 1}"));
}

void CzmqDemoDlg::OnBnClickedBtnTerminate()
{
    // TODO: 在此添加控件通知处理程序代码
    sendResult("{\"jsonrpc\": \"2.0\", \"method\": \"terminate\"}", 
        strlen("{\"jsonrpc\": \"2.0\", \"method\": \"terminate\"}"));
}

LRESULT CzmqDemoDlg::OnCallComing(WPARAM wParam, LPARAM lParam) 
{ 
    // TODO: 处理用户自定义消息
    int id = (int)wParam;
    int rt = ::AfxMessageBox("远端来电是否接听", MB_YESNO);
    if(rt == IDYES){
        sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"LinphoneCallConnected\", \"id\": %d}", id);
    }else{
        sprintf(g_cSendBuff, "{\"jsonrpc\": \"2.0\", \"result\": \"reject call\", \"id\": %d}", id);
    }
    sendResult(g_cSendBuff, strlen(g_cSendBuff));
    return 0; 
}

