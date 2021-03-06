
// PayServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PayServer.h"
#include "PayServerDlg.h"
#include "afxdialogex.h"
#include "..\common\common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

DWORD WINAPI RecvThread(LPVOID lpParam);
UINT WINAPI ClientThread(LPVOID lpParam);

CPayServerDlg* g_PaySerDlg=NULL;
class CAboutDlg : public CDialogEx
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

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPayServerDlg 对话框
CPayServerDlg::CPayServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPayServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_SocketHandle=INVALID_HANDLE_VALUE;
	m_bExit = FALSE;
	m_mutex = CreateMutex(NULL, FALSE, NULL);
	m_sendMutex=CreateMutex(NULL, FALSE, NULL);
}

void CPayServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListBox);
	DDX_Control(pDX, IDC_LIST3, m_listCtrl);
	DDX_Control(pDX, IDC_STA_IP, m_staIP);
	DDX_Control(pDX, IDC_CK_STARTRUN, m_ckRun);
	DDX_Control(pDX, IDC_STA, m_NowSta);
}

BEGIN_MESSAGE_MAP(CPayServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SOCKMSG, &CPayServerDlg::OnSockMsg)
	ON_MESSAGE(WM_SHOWTASK, &CPayServerDlg::OnShowTask)
	ON_COMMAND(ID_S_SHOW, OnShow)
	ON_COMMAND(ID_S_CLOSE, OnClose)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_HIDE, &CPayServerDlg::OnBnClickedBtnHide)
	ON_BN_CLICKED(IDC_CK_STARTRUN, &CPayServerDlg::OnBnClickedCkStartrun)
END_MESSAGE_MAP()


// CPayServerDlg 消息处理程序

BOOL CPayServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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
	g_PaySerDlg=this;

	InitListCtrl();
	//初始化socket
	AddString(L"正在初始化网络程序...");
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2,2),&wsadata)!=0)
	{
		AddString(L"InitSocket error!");
	}
	else if (g_Globle.InitGloble())
	{
		USES_CONVERSION;
		CString str;
		str.Format(L"IP：%s    端口：%d",A2W(g_Globle.m_LocalIP.c_str()),g_Globle.m_TcpPort);
		m_staIP.SetWindowTextW(str);
		//开启服务监听
		m_SocketHandle = CreateThread(NULL,0,RecvThread,this,0,NULL);
		//开始定时刷新客户端列表
		SetTimer(1,2000,NULL);
	}
	InitChechBox();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPayServerDlg::InitChechBox()
{
	HKEY hKey;
	CString lpRun = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	long lRet = RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_ALL_ACCESS, &hKey);
	if (lRet != ERROR_SUCCESS)
		MessageBox(_T("打开启动项失败"));

	TCHAR pFileName[MAX_PATH] = {};
	GetModuleFileName(NULL, pFileName, MAX_PATH);
	//判断是否已经设置开机启动
	TCHAR PowerBoot[MAX_PATH] = {};
	DWORD nLongth = MAX_PATH;
	long result = RegGetValue(hKey, NULL, _T("PayServer"), RRF_RT_REG_SZ, 0, PowerBoot, &nLongth);
	if (result == ERROR_SUCCESS)
	{
		m_ckRun.SetCheck(TRUE);
	}
	RegCloseKey(hKey); 
}

void CPayServerDlg::InitListCtrl()
{
	TCHAR rgtsz[4][10] = {_T("账号"), _T("端口") , _T("名称"),_T("上次心跳时间")};
	LV_COLUMN lvcolumn;
	CRect rect;
	m_listCtrl.GetWindowRect(&rect);
	for (int i=0;i<4;i++)
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT
			| LVCF_WIDTH | LVCF_ORDER;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = rgtsz[i];
		lvcolumn.iSubItem = i;
		lvcolumn.iOrder = i;
		if(i==0)
		    lvcolumn.cx = 100;
		else if(i==1)
			lvcolumn.cx = 70;
		else
			lvcolumn.cx = ((rect.Width()-170)/2) -2;
		m_listCtrl.InsertColumn(i, &lvcolumn);
	}
	m_listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_listCtrl.SetBkColor(RGB(211 ,211 ,211));
	m_listCtrl.SetTextBkColor(RGB(102 ,205, 170));
}

void CPayServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_MINIMIZE)
	{
		Hide();
		return;
	}

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

void CPayServerDlg::OnShow()
{
	this->ShowWindow(SW_SHOWNORMAL);
	this->SetForegroundWindow();
	Shell_NotifyIcon(NIM_DELETE, &m_nid); // 托盘图标不显示
}

void CPayServerDlg::OnClose()
{
	PostMessage(WM_CLOSE);
}

void CPayServerDlg::Hide()
{
	m_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_nid.hWnd = this->m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_nid.uCallbackMessage = WM_SHOWTASK;
	m_nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	lstrcpy(m_nid.szTip, _T("payServer"));
	Shell_NotifyIcon(NIM_ADD, &m_nid); 
	ShowWindow(SW_HIDE);
}
// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPayServerDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPayServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CPayServerDlg::OnSockMsg(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
	case SOCK_MSG_OK:
		{
			AddString(L"服务启动成功，等待用户连接...");
			break;
		}
	case SOCK_ERROR_CREATESOCK:
		{
			AddString(L"服务启动失败:create socket error!");
			break;
		}
	case SOCK_ERROR_BIND:
		{
			AddString(L"服务启动失败:socket bind error!");
			break;
		}
	case SOCK_ERROR_CREATEEVENT:
		{
			AddString(L"服务启动失败:create event error!");
			break;
		}
	case SOCK_MSG_CLOSE:
		{
			AddString(L"服务已经关闭");
			break;
		}
	case SOCK_MSG_TIMEOUT:
		{
			
			break;
		}
	}
	return TRUE;
}

LRESULT CPayServerDlg::OnShowTask(WPARAM wParam, LPARAM lParam)
{
	if (wParam != IDR_MAINFRAME)
		return 1;
	if (WM_LBUTTONDBLCLK == lParam)
	{
		this->ShowWindow(SW_SHOWNORMAL);
		this->SetForegroundWindow();
		Shell_NotifyIcon(NIM_DELETE, &m_nid); // 托盘图标不显示
	}
	if (lParam == WM_RBUTTONDOWN)
	{
		//右击弹出托盘菜单
		CMenu menu;
		menu.LoadMenu(IDR_MENU1);
		CMenu *pPopUp = menu.GetSubMenu(0);
		CPoint pt;
		GetCursorPos(&pt);
		SetForegroundWindow();
		pPopUp->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
		PostMessage(WM_NULL, 0, 0);
	}
	return 0;
}

void CPayServerDlg::DeleteClient(Node* pnode)
{
	WaitForSingleObject(m_mutex, INFINITE); 

	vector<Node*>::iterator it;
	for(it=m_vNode.begin();it!=m_vNode.end();)
	{
		if ((*it)->s == pnode->s)
		{
			it=m_vNode.erase(it);
			break;
		}
		else
		{
			++it;
		}
	}
	
	delete pnode;
	pnode=NULL;

	CString str;
	str.Format(L"当前连接数：%d",m_vNode.size());
	SetDlgItemText(IDC_STA,str);

	ReleaseMutex(m_mutex); 
}

void CPayServerDlg::AddString(CString str)
{
	
	CString str2,strTime;
	SYSTEMTIME st;
	GetLocalTime(&st);
	strTime.Format(L"%4d/%02d/%02d %02d:%02d:%02d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	str2.Format(L"%s：  %s",strTime, str);
	m_ListBox.AddString(str2);
}

bool CPayServerDlg::AddClientList(SOCKET s,sockaddr_in addr)
{
	USES_CONVERSION;
	Node* node = new Node;
	node->s=s;
	node->Addr=addr;

	/*vector<Node*>::iterator it;
	for(it=m_vNode.begin();it!=m_vNode.end();)
	{
		if ((*it)->s == node->s)
		{
			break;
		}
		else
		{
			++it;
		}
	}*/

	HANDLE hThread = NULL;
	unsigned int ThreadID = 0;
	hThread = (HANDLE) _beginthreadex( NULL, 0, ClientThread, node, CREATE_SUSPENDED,&ThreadID);
	if (hThread==NULL)
	{
		return false;
	}
	else
	{
		node->nThreadID=ThreadID;
		node->hHandle = hThread;
		/*WaitForSingleObject(m_mutex, INFINITE);
		m_vNode.push_back(node);
		ReleaseMutex(m_mutex); 

		CString str;
		str.Format(L"%d 连入",ThreadID);
		AddString(str);

		str.Format(L"当前连接数：%d",m_vNode.size());
		SetDlgItemText(IDC_STA,str);*/
		ResumeThread(hThread);
	}
	return true;
}

void CPayServerDlg::DoRun(string strData,Json::Value& js,Node* pnode)
{
	try
	{
		bool bRet = false;
		USES_CONVERSION;
		Json::Reader r;
		Json::Value root;
		r.parse(strData,root);

		Json::FastWriter writer;  
		string temp = writer.write(root);

		EM_SOCK_CMD cmd = (EM_SOCK_CMD)root[CONNECT_CMD].asInt();
		js[CONNECT_CMD]=cmd;
		switch (cmd)
		{
		case SOCK_CMD_HEART:
			{
				CString strHdTime;
				SYSTEMTIME st;
				GetLocalTime(&st);
				strHdTime.Format(L"%4d/%02d/%02d %02d:%02d:%02d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
			    pnode->strLastHeartTime = strHdTime;
				CString strUser = A2T(root[HEARTMSG[EM_HEART_USER]].asCString());
				CString strName = A2T(root[HEARTMSG[EM_HEART_NAME]].asCString());
				pnode->strUser = strUser;
				pnode->strName = strName;
			}
			break;
		case SOCK_CMD_GET_USER:
			{
				bRet=theApp.m_dbData->GetUsers(js);
			}
			break;
		case SOCK_CMD_JUDGE_USER:
			{
				CString strName = A2T(root[CMD_JUDGEUSER[EM_JUDGE_USER_NAME]].asCString());
				bRet=theApp.m_dbData->_JudgeUser(strName,js);
			}
			break;
		case SOCK_CMD_ADD_USER:
			{
				CString strName = A2T(root[CMD_ADDUSER[EM_ADD_USER_NAME]].asCString());
				CString strPwd = A2T(root[CMD_ADDUSER[EM_ADD_USER_PWD]].asCString());
				USER_TYPE type = (USER_TYPE)root[CMD_ADDUSER[EM_ADD_USER_TYPE]].asInt();
				bRet=theApp.m_dbData->AddUser(strName,strPwd,type);
			}
			break;
		case SOCK_CMD_DEL_USER:
			{
				CString strName = A2T(root[CMD_DELUSER[EM_JUDGE_USER_NAME]].asCString());
				bRet=theApp.m_dbData->DelUser(strName);
			}
			break;
		case SOCK_CMD_INIT:
			{
				bRet=theApp.m_dbData->InitData();
			}
			break;
		case SOCK_CMD_CHECKPWD:
			{
				CString strName = A2T(root[CMD_CHECKPWD[EM_CHECKPWD_NAME]].asCString());
				CString strPass = A2T(root[CMD_CHECKPWD[EM_CHECKPWD_PWD]].asCString());
				bRet=theApp.m_dbData->CheckPwd(strName,strPass);
			}
			break;
		case SOCK_CMD_MDFPWD:
			{
				CString strName = A2T(root[CMD_MDFPWD[EM_MDFPWD_NAME]].asCString());
				CString strPass = A2T(root[CMD_MDFPWD[EM_MDFPWD_PWD]].asCString());
				bRet=theApp.m_dbData->ModifyPwd(strName,strPass);
			}
			break;
		case SOCK_CMD_LOGIN:
			{
				CString strUser = A2T(root[CMD_LOGIN[EM_LOGIN_NAME]].asCString());
				CString strPass = A2T(root[CMD_LOGIN[EM_LOGIN_PASS]].asCString());
				int show_pass = root[CMD_LOGIN[EM_LOGIN_SHOWPASS]].asInt();
				//判断该账号是否已经登录
				bool bLogind = false;
				for (int i = 0;i<m_vNode.size();i++)
				{
					if (m_vNode[i]->strUser == strUser)
					{
						//已经登录
						bLogind = true;
						js[CMD_LOGINMSG[EM_LOGINMSG_BOK]] = false;
						js[CMD_LOGINMSG[EM_LOGINMSG_LOGIND]] = bLogind;
						bRet = true;
						break;
					}
				}
				if(!bLogind)
					bRet=theApp.m_dbData->Login(strUser,strPass,show_pass,js);
			}
			break;
		case SOCK_CMD_GET_BOOK:
			{
				CString strKeyWord = A2T(root[CMD_GETBOOK[GETBOOK_KEYWORD]].asCString());
				BOOK_RK em_rk = (BOOK_RK)root[CMD_GETBOOK[GETBOOK_RKTYPE]].asInt();
				EM_DATE_TYPE em_date = (EM_DATE_TYPE)root[CMD_GETBOOK[GETBOOK_DATE]].asInt();
				int n = root[CMD_GETBOOK[GETBOOK_NSTART]].asInt();
				int number = root[CMD_GETBOOK[GETBOOK_NUMBER]].asInt();
				bRet=theApp.m_dbData->GetBooks(js,strKeyWord,em_rk,em_date,n,number);
			}
			break;
		case SOCK_CMD_GET_SAMPLE_BOOK:
			{
				BOOK_RK rkType = (BOOK_RK)root[CMD_GETBOOK[GETBOOK_RKTYPE]].asInt();
				bRet=theApp.m_dbData->GetSampleBooks(js,rkType);
			}
			break;
		case SOCK_CMD_DEL_BOOK:
			{
				CString strBookID = A2T(root[CMD_DELBOOK[EM_DEL_BOOK_ID]].asCString());
				bRet=theApp.m_dbData->DelBook(strBookID);
			}
			break;
		case SOCK_CMD_JUDGE_BOOK:
			{
				CString strName = A2T(root[CMD_JUDGEBOOK[EM_JUDGE_BOOK_NAME]].asCString());
				bRet=theApp.m_dbData->_JudgeBook(strName,js);
			}
			break;
		case SOCK_CMD_ADD_BOOK:
		case SOCK_CMD_MDF_BOOK:
			{
				CString strBookID = A2T(root[CMD_BOOKMSG[BOOKMSG_ID]].asCString());
				CString strName = A2T(root[CMD_BOOKMSG[BOOKMSG_NAME]].asCString());
				CString strCbs = A2T(root[CMD_BOOKMSG[BOOKMSG_CBS]].asCString());
				CString strDate = A2T(root[CMD_BOOKMSG[BOOKMSG_DATE]].asCString());
				int nBc1 = root[CMD_BOOKMSG[BOOKMSG_BC1]].asInt();
				int nBc2 = root[CMD_BOOKMSG[BOOKMSG_BC2]].asInt();
				int nSize1 = root[CMD_BOOKMSG[BOOKMSG_SIZE1]].asInt();
				int nSize2 = root[CMD_BOOKMSG[BOOKMSG_SIZE2]].asInt();
				int nKb = root[CMD_BOOKMSG[BOOKMSG_KB]].asInt();
				double fYz = root[CMD_BOOKMSG[BOOKMSG_YZ]].asDouble();
				int nYs = root[CMD_BOOKMSG[BOOKMSG_YS]].asInt();
				int nBc = root[CMD_BOOKMSG[BOOKMSG_BC]].asInt();
				double fLs = root[CMD_BOOKMSG[BOOKMSG_LS]].asDouble();
				int yzType = root[CMD_BOOKMSG[BOOKMSG_B_TYPE]].asInt();
				int zyType = root[CMD_BOOKMSG[BOOKMSG_ZY_TYPE]].asInt();
				int rkType = root[CMD_BOOKMSG[BOOKMSG_RK_TYPE]].asInt();
				CString strMsg = A2T(root[CMD_BOOKMSG[BOOKMSG_MSG]].asCString());

				if(cmd == SOCK_CMD_ADD_BOOK)
					bRet=theApp.m_dbData->AddBook(strBookID, strName,strCbs,strDate,nBc1,nBc2, nSize1,nSize2,nKb, fYz,nYs,nBc,fLs,yzType,zyType,rkType,strMsg);
				else if(cmd == SOCK_CMD_MDF_BOOK)
					bRet=theApp.m_dbData->ModifyBook(strBookID, strName,strCbs,strDate,nBc1,nBc2, nSize1,nSize2,nKb, fYz,nYs,nBc,fLs,yzType,zyType,rkType,strMsg);
			}
			break;
		case SOCK_CMD_GET_PROJECT:
			{
				PRO_STAFF_TYPE type = (PRO_STAFF_TYPE)root[CMD_GETPRO[EM_GETPRO_BWRITE]].asInt();
				bRet=theApp.m_dbData->GetProjectLists(js,type);
			}
			break;
		case SOCK_CMD_DEL_PROJECT:
			{
				int nProID = root[CMD_DELPROJECT[EM_DEL_PROJECT_ID]].asInt();
				bRet=theApp.m_dbData->DelOneProject(nProID);
			}
			break;
		case SOCK_CMD_JUDGE_PROJECT:
			{
				CString strProName = A2T(root[CMD_JUDGEBOOK[EM_JUDGE_PROJECT_NAME]].asCString());
				bRet=theApp.m_dbData->_JudgePro(strProName,js);
			}
			break;
		case SOCK_CMD_ADD_PROJECT:
		case SOCK_CMD_MDF_PROJECT:
			{
				CString strName = A2T(root[CMD_PROMSG[EM_PROMSG_NAME]].asCString());
				PRO_NUM_TYPE numType = (PRO_NUM_TYPE)root[CMD_PROMSG[EM_PROMSG_NUM_TYPE]].asInt();
				PRO_STAFF_TYPE bWrite = (PRO_STAFF_TYPE)root[CMD_PROMSG[EM_PROMSG_BWRITE]].asInt();
				if(cmd == SOCK_CMD_ADD_PROJECT)
					bRet=theApp.m_dbData->AddProject(strName,numType,bWrite);
				else if(cmd == SOCK_CMD_MDF_PROJECT)
				{
					int nID = root[CMD_PROMSG[EM_PROMSG_ID]].asInt();
					bRet=theApp.m_dbData->ModifyProject(nID,strName,numType,bWrite);
				}
			}
			break;
		case SOCK_CMD_SAVE_PRONDEX:
			{
				bRet=theApp.m_dbData->SaveProNdex(root);
			}
			break;
		case SOCK_CMD_GET_PROGRESS:
			{
				CString strBookID = A2T(root[CMD_GETPROGRESS[EM_GETPROGRESS_BOOKID]].asCString());
				bRet=theApp.m_dbData->_GetProgress(js,root,strBookID);
			}
			break;
		case SOCK_CMD_GET_STAFFWRITE:
			{
				CString strBookID = A2T(root[CMD_GETPROGRESS[EM_GETPROGRESS_BOOKID]].asCString());
				int nProID = root[CMD_GETPROGRESS[EM_GETPROGRESS_PROID]].asInt();
				bRet=theApp.m_dbData->GetStaffWrite(js,strBookID,nProID);
			}
			break;
		case SOCK_CMD_SAVE_STAFFWRITE:
			{
				CString strBookID = A2T(root[CMD_GETPROGRESS[EM_GETPROGRESS_BOOKID]].asCString());
				int nProID = root[CMD_GETPROGRESS[EM_GETPROGRESS_PROID]].asInt();
				double number = root[CMD_RetType[EM_CMD_RETYPE_VALUE]].asDouble();
				bRet=theApp.m_dbData->SaveStaffWrite(strBookID,nProID,number);
			}
			break;
		case SOCK_CMD_GET_STAFF:
			{
				CString strKeyWord = A2T(root[CMD_GETSTAFF[EM_GET_STAFF_KEYWORD]].asCString());
				int n = root[CMD_GETSTAFF[EM_GET_STAFF_NSTART]].asInt();
				int number = root[CMD_GETSTAFF[EM_GET_STAFF_NUMBER]].asInt();
				bRet=theApp.m_dbData->GetStaffs(strKeyWord,js,n,number);
			}
			break;
		case SOCK_CMD_GET_SAMPLE_STAFF:
			{
				bRet=theApp.m_dbData->GetSampleStaffs(js);
			}
			break;
		case SOCK_CMD_DEL_STAFF:
			{
				CString strStaffID = A2T(root[CMD_DELSTAFF[EM_DEL_STAFF_ID]].asCString());
				bRet=theApp.m_dbData->DelStaff(strStaffID);
			}
			break;
		case SOCK_CMD_GET_DPAY:
			{
				CString strStaffID = A2T(root[CMD_STAFFMSG[EM_STAFF_MSG_STAFFID]].asCString());
				bRet=theApp.m_dbData->GetDPay(strStaffID,js);
			}
			break;
		case SOCK_CMD_JUDGE_STAFF:
			{
				CString strIdcard = A2T(root[CMD_JUDGESTAFF[EM_JUDGE_STAFF_IDCARD]].asCString());
				bRet=theApp.m_dbData->_JudgeStaff(strIdcard,js);
			}
			break;
		case SOCK_CMD_ADD_STAFF:
		case SOCK_CMD_MDF_STAFF:
			{
				CString strName = A2T(root[CMD_STAFFMSG[EM_STAFF_MSG_NAME]].asCString());
				CString strSex = A2T(root[CMD_STAFFMSG[EM_STAFF_MSG_SEX]].asCString());
				int age = root[CMD_STAFFMSG[EM_STAFF_MSG_AGE]].asInt();
				CString strStaffID = A2T(root[CMD_STAFFMSG[EM_STAFF_MSG_STAFFID]].asCString());
				CString strIdCard = A2T(root[CMD_STAFFMSG[EM_STAFF_MSG_IDCARD]].asCString());
				CString strTel = A2T(root[CMD_STAFFMSG[EM_STAFF_MSG_TEL]].asCString());
				STAFF_TYPE type = (STAFF_TYPE)root[CMD_STAFFMSG[EM_STAFF_MSG_TYPE]].asInt();
				int sort = root[CMD_STAFFMSG[EM_STAFF_MSG_SORT]].asInt();
				double fDaypay = root[CMD_STAFFMSG[EM_STAFF_MSG_DAYPAY]].asDouble();
				if(cmd == SOCK_CMD_ADD_STAFF)
				    bRet=theApp.m_dbData->AddStaff(strName,strSex,age,strStaffID,strIdCard,strTel,type,sort,fDaypay);
				else if(cmd == SOCK_CMD_MDF_STAFF)
					bRet=theApp.m_dbData->ModifyStaff(strName,strSex,age,strStaffID,strIdCard,strTel,type,sort,fDaypay);
			}
			break;
		case SOCK_CMD_GET_DAIPAY:
			{
				bRet=theApp.m_dbData->_GetDaiPay(js);
			}
			break;
		case SOCK_CMD_SET_DAIPAY:
			{
				STU_DAI_PAY stu;
				stu.strA_w = A2T(root[CMD_DAIPAY[EM_DAI_PAY_A_W]].asCString());
				stu.strSf_w = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Sf_w]].asCString());
				stu.strTd_w = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Td_w]].asCString());

				stu.strA_2 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_2]].asCString());
				stu.strSf_2 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Sf_2]].asCString());
				stu.strTd_2 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Td_2]].asCString());

				stu.strA_2_5 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_2_5]].asCString());
				stu.strSf_2_5 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Sf_2_5]].asCString());
				stu.strTd_2_5 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Td_2_5]].asCString());

				stu.strA_5_9 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_5_9]].asCString());
				stu.strSf_5_9 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Sf_5_9]].asCString());
				stu.strTd_5_9 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Td_5_9]].asCString());

				stu.strA_10 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_10]].asCString());
				stu.strSf_10 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Sf_10]].asCString());
				stu.strTd_10 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Td_10]].asCString());

				stu.strA_18 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_18]].asCString());
				stu.strSf_18 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Sf_18]].asCString());
				stu.strTd_18 = A2T(root[CMD_DAIPAY[EM_DAI_PAY_Td_18]].asCString());

				bRet=theApp.m_dbData->SaveDaiPay(stu);
			}
			break;
		case SOCK_CMD_SET_ZHEYEPAY:
			{
				STU_ZHEYE_PAY stu;
				stu.strAQ4 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_AQ4]].asCString();
				stu.strA4 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_A4]].asCString();
				stu.strAD3 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_AD3]].asCString();
				stu.strA3 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_A3]].asCString();
				stu.strA2 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_A2]].asCString();

				stu.str_sf_Q4 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_sf_Q4]].asCString();
				stu.str_sf_4 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_sf_4]].asCString();
				stu.str_sf_D3 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_sf_D3]].asCString();
				stu.str_sf_3 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_sf_3]].asCString();
				stu.str_sf_2 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_sf_2]].asCString();

				stu.str_td_Q4 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_td_Q4]].asCString();
				stu.str_td_4 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_td_4]].asCString();
				stu.str_td_D3 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_td_D3]].asCString();
				stu.str_td_3 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_td_3]].asCString();
				stu.str_td_2 = root[CMD_ZHEYEPAY[EM_ZHEYE_PAY_td_2]].asCString();
			    bRet=theApp.m_dbData->SaveZheYePay(stu);
			}
			break;
		case SOCK_CMD_GET_ZHEYEPAY:
			{
				bRet=theApp.m_dbData->_GetZheYePay(js);
			}
			break;
		case SOCK_CMD_GET_DYPAY:
			{
				bRet=theApp.m_dbData->_GetDyPay(js);
			}
			break;
		case SOCK_CMD_SET_DYPAY:
			{
				CString strdown,strup;
				strdown = root[CMD_DYPAY[EM_DY_PAY_DOWN]].asCString();
				strup = root[CMD_DYPAY[EM_DY_PAY_UP]].asCString();
				bRet=theApp.m_dbData->SaveDianyePay(strdown,strup);
			}
			break;
		case SOCK_CMD_GET_OTHERPAY:
			{
				int nProID = root[CMD_OTHERPAY[EM_OTHER_PAY_PROID]].asInt();
				bRet=theApp.m_dbData->_GetOtherPay(js,nProID);
			}
			break;
		case SOCK_CMD_SET_OTHERPAY:
			{
				vector<OTHER_PRO_PAY> vec;
				int nProID = root[CMD_OTHERPAY[EM_OTHER_PAY_PROID]].asInt();
				if (root.isMember(CMD_RetType[EM_CMD_RETYPE_VALUE]))
				{
					if (root[CMD_RetType[EM_CMD_RETYPE_VALUE]].isArray())
					{
						Json::Value vle = root[CMD_RetType[EM_CMD_RETYPE_VALUE]];
						for (int i=0;i<vle.size();i++)
						{
							OTHER_PRO_PAY pay;
							pay.strBookID=vle[i][CMD_OTHERPAYMSG[EM_OTHER_PAY_MSG_BOOKID]].asCString();
							pay.strPay=vle[i][CMD_OTHERPAYMSG[EM_OTHER_PAY_MSG_PAY]].asCString();
							vec.push_back(pay);
						}
					}
				}
				bRet = theApp.m_dbData->SaveOtherPay(nProID,vec);
			}
			break;
		case SOCK_CMD_SET_OTHERALLBOOKPAY:
			{
				CString strPay;
				int nProID = root[CMD_PROMSG[EM_PROMSG_ID]].asInt();
				strPay = root[CMD_PROMSG[EM_PROMSG_PAY]].asCString();
				bRet=theApp.m_dbData->SaveOtherAllBookPay(nProID,strPay);
			}
			break;
		case SOCK_CMD_GET_DETAILS:
			{
				CString strBookID;
				vector<PROJECT_STU> vec;
				strBookID = root[CMD_GETDETAILS[EM_GET_DETAILS_BOOKID]].asCString();
				if (root.isMember(CMD_GETDETAILS[EM_GET_DETAILS_PROIDS]))
				{
					if (root[CMD_GETDETAILS[EM_GET_DETAILS_PROIDS]].isArray())
					{
						Json::Value vle = root[CMD_GETDETAILS[EM_GET_DETAILS_PROIDS]];
						for (int i=0;i<vle.size();i++)
						{
							PROJECT_STU stu;
							stu.nID=vle[i][CMD_GETDETAILS[EM_GET_DETAILS_PROID]].asInt();
							stu.ndex=vle[i][CMD_GETDETAILS[EM_GET_DETAILS_NDEX]].asInt();
							vec.push_back(stu);
						}
					}
				}
				bRet=theApp.m_dbData->_GetDetails(js,vec,strBookID);
			}
			break;
		case SOCK_CMD_GET_DAYPAY:
			{
				CString strStaffID,strDate;
				strStaffID = root[CMD_GETDAYPAY[EM_GET_DAYPAY_STAFFID]].asCString();
				strDate = root[CMD_GETDAYPAY[EM_GET_DAYPAY_DATE]].asCString();
				bRet=theApp.m_dbData->_GetDayPay(js,strStaffID,strDate);
			}
			break;
		case SOCK_CMD_GET_DAYPAY_LIST:
			{
				bRet=theApp.m_dbData->_GetDayPayList(js,root);
			}
			break;
		case SOCK_CMD_GET_MPAY:
			{
				DWORD time=0;
				bRet=theApp.m_dbData->_GetMouthPay(js,root,time);
			}
			break;
		case SOCK_CMD_DEL_DAYPAY:
			{
				CString strStaffID,strDate;
				strStaffID = root[CMD_GETDAYPAY[EM_GET_DAYPAY_STAFFID]].asCString();
				strDate = root[CMD_GETDAYPAY[EM_GET_DAYPAY_DATE]].asCString();
				bRet=theApp.m_dbData->DeleteDayPay(strStaffID,strDate);
			}
			break;
		case SOCK_CMD_SAVE_DAYPAY:
			{
				CString strStaffID,strDate;
				strStaffID = root[CMD_GETDAYPAY[EM_GET_DAYPAY_STAFFID]].asCString();
				strDate = root[CMD_GETDAYPAY[EM_GET_DAYPAY_DATE]].asCString();
				vector<DAYPAY> vec;
				if (root.isMember(CMD_RetType[EM_CMD_RETYPE_VALUE]))
				{
					if (root[CMD_RetType[EM_CMD_RETYPE_VALUE]].isArray())
					{
						Json::Value vle = root[CMD_RetType[EM_CMD_RETYPE_VALUE]];
						for (int i=0;i<vle.size();i++)
						{
							DAYPAY stu;
							stu.type=(DAYPAY_TYPE)vle[i][DAYPAYMSG[EM_DAYPAY_MSG_TYPE]].asInt();
							if (stu.type == DAYPAY_TYPE_DAY)
							{
								stu.strPayDay = vle[i][DAYPAYMSG[EM_DAYPAY_MSG_PAYDAY]].asCString();
								stu.strDays = vle[i][DAYPAYMSG[EM_DAYPAY_MSG_DAYS]].asCString();
							}
							else
							{
								stu.proID = vle[i][DAYPAYMSG[EM_DAYPAY_MSG_PROID]].asInt();
								stu.strBookID = vle[i][DAYPAYMSG[EM_DAYPAY_MSG_BOOKID]].asCString();
								stu.pay = vle[i][DAYPAYMSG[EM_DAYPAY_MSG_PAY]].asCString();
								stu.number = vle[i][DAYPAYMSG[EM_DAYPAY_MSG_NUMBER]].asDouble();
								stu.strProName = vle[i][DAYPAYMSG[EM_DAYPAY_MSG_PRONAME]].asCString();
								stu.strBookName = vle[i][DAYPAYMSG[EM_DAYPAY_MSG_BOOKNAME]].asCString();
							}
							stu.money = vle[i][DAYPAYMSG[EM_DAYPAY_MSG_MONEY]].asCString();
							vec.push_back(stu);
						}
						bRet=theApp.m_dbData->AddDayPay(strStaffID,vec,strDate);
					}
				}
			}
			break;
		case SOCK_CMD_GET_PAY:
			{
				int nProID,nItem;
				CString strStaffID,strBookID;
				strStaffID = root[GETPAYMSG[EM_GET_PAY_STAFFID]].asCString();
				strBookID = root[GETPAYMSG[EM_GET_PAY_BOOKID]].asCString();
				nProID = root[GETPAYMSG[EM_GET_PAY_PROID]].asInt();
				nItem = root[GETPAYMSG[EM_GET_PAY_NITEM]].asInt();
				bRet=theApp.m_dbData->_GetOnePay(js,strStaffID,nProID,strBookID);
				js[GETPAYMSG[EM_GET_PAY_NITEM]] = nItem;
			}
			break;
		default:
			{
			}
		}
		if (bRet)
			js[CMD_RetType[EM_CMD_RETYPE_RESULT]]=NET_CMD_SUC;
		else
			js[CMD_RetType[EM_CMD_RETYPE_RESULT]]=NET_CMD_FAIL;
	}
	catch (...)
	{
		js[CMD_RetType[EM_CMD_RETYPE_RESULT]]=NET_CMD_FAIL;
	}

}

void CPayServerDlg::SendTo(SOCKET sock,Json::Value js)
{
	WaitForSingleObject(m_sendMutex, INFINITE);
	do 
	{
		Json::FastWriter writer;  
		string strData = writer.write(js);
		string tr = strData.substr(strData.length()-1,1);
		if (tr=="\n")
		{
			strData = strData.substr(0,strData.length()-1);
		}

		long AllLen = 0;
		char* sendBuf = g_Globle.CombineSendData(strData,AllLen);
		if(sendBuf == NULL)
		{
			AddString(L"sendBuf is null!");
			break;
		}
		if (AllLen >= MAXBUFFLEN)
		{
			CString str;
			str.Format(L"发送报文长度：%d,已超出最大接收长度，发送失败！",AllLen);
			g_PaySerDlg->AddString(str);
			break;
		}

		DWORD NumberOfBytesSent = 0;
		DWORD dwBytesSent = 0;
		WSABUF Buffers;
		int  Ret = 0;

		if (sock)
		{
			do
			{
				Buffers.buf = sendBuf;
				Buffers.len = AllLen;//strlen(szResponse)遇到0x00会中断
				Ret = WSASend(sock,&Buffers,1,&NumberOfBytesSent,0,0,NULL);  
				if(SOCKET_ERROR != Ret)
					dwBytesSent += NumberOfBytesSent;
				else
				{
					CString str;
					str.Format(L"send error：%s",strData.c_str());
					AddString(str);
					break;
				}
			}
			while((dwBytesSent < AllLen) && SOCKET_ERROR != Ret);
		}
		delete[] sendBuf;
	} while (0);
	ReleaseMutex(m_sendMutex); 
}

DWORD WINAPI RecvThread(LPVOID lpParam)
{
	CPayServerDlg* pThis = (CPayServerDlg*)lpParam;

	SOCKET Ser_socket = INVALID_SOCKET;
	while (!pThis->m_bExit)
	{
		Ser_socket = WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED); //使用事件重叠的套接字
		if (Ser_socket==INVALID_SOCKET)
		{
			pThis->PostMessageW(WM_SOCKMSG,NULL,SOCK_ERROR_CREATESOCK);
			Sleep(2000);
			continue; 
		}
		//初始化本服务器的地址
		sockaddr_in LocalAddr;
		LocalAddr.sin_addr.S_un.S_addr = inet_addr(g_Globle.m_LocalIP.c_str());
		LocalAddr.sin_family = AF_INET;
		LocalAddr.sin_port = htons(g_Globle.m_TcpPort);

		int Ret = bind(Ser_socket,(sockaddr*)&LocalAddr,sizeof(LocalAddr));
		if (Ret==SOCKET_ERROR)
		{
			pThis->PostMessageW(WM_SOCKMSG,NULL,SOCK_ERROR_BIND);
			Sleep(2000);
			continue;
		}
		break;
	}

	listen(Ser_socket,20);

	//创建事件
	WSAEVENT wsa_event = WSACreateEvent();
	if (wsa_event==WSA_INVALID_EVENT)
	{
		pThis->PostMessageW(WM_SOCKMSG,NULL,SOCK_ERROR_CREATEEVENT);
		closesocket(Ser_socket);
		CloseHandle(wsa_event);    
		return 0;
	}

	pThis->PostMessageW(WM_SOCKMSG,NULL,SOCK_MSG_OK);
	WSAEventSelect(Ser_socket,wsa_event,FD_ACCEPT);
	WSANETWORKEVENTS NetWorkEvent;
	sockaddr_in ClientAddr;
	int nLen = sizeof(ClientAddr);
	DWORD dwIndex = 0;
	while (!pThis->m_bExit)
	{
		dwIndex = WSAWaitForMultipleEvents(1,&wsa_event,FALSE,1000,FALSE);
		dwIndex = dwIndex - WAIT_OBJECT_0;
		if (dwIndex==WSA_WAIT_TIMEOUT||dwIndex==WSA_WAIT_FAILED)
		{
			continue;
		}
		//对网络事件进行判断
		WSAEnumNetworkEvents(Ser_socket,wsa_event,&NetWorkEvent);
		ResetEvent(&wsa_event);
		if (NetWorkEvent.lNetworkEvents == FD_ACCEPT)
		{
			if (NetWorkEvent.iErrorCode[FD_ACCEPT_BIT]==0)
			{
				SOCKET sClient = WSAAccept(Ser_socket, (sockaddr*)&ClientAddr, &nLen, NULL, NULL);
				if (sClient==INVALID_SOCKET)
				{
					continue;
				}
				else
				{
					//如果接收成功则把用户的所有信息存入链表
					if (!pThis->AddClientList(sClient,ClientAddr))
					{
						closesocket(sClient);
					}
				}
			}
		}
	}
	closesocket(Ser_socket);
	WSACloseEvent(wsa_event);
	pThis->PostMessageW(WM_SOCKMSG,NULL,SOCK_MSG_CLOSE);
	return 0;
}

UINT WINAPI ClientThread(LPVOID lpParam)
{
	CString str;
	Node* pnode = (Node*)lpParam;
	SOCKET sClient = pnode->s;
	WSAEVENT Event = WSACreateEvent();
	WSANETWORKEVENTS NetWorkEvent;

	if(Event == WSA_INVALID_EVENT)
	{
		g_PaySerDlg->DeleteClient(pnode);
		return -1;
	}
	int Ret = WSAEventSelect(sClient, Event, FD_READ | FD_WRITE | FD_CLOSE);

	int  waitTime=0;
	long maxlen = MAXBUFFLEN;
	char* szRequest=new char[maxlen];
	memset(szRequest,0,maxlen);
	DWORD NumberOfBytesRecvd;
	DWORD dwIndex = 0;
	WSABUF Buffers;
	DWORD dwBufferCount = 1;
	DWORD Flags = 0;
	Buffers.buf = szRequest;
	Buffers.len = 5;//5为数据头长度
	//数据长度
	long dataLen=0;
	long   revLen =0;
	bool bOnePack = false;//第一个报文是否正常,不正常说明是异常连接
	while (!g_PaySerDlg->m_bExit)
	{
		//等待网络事件触发,m_HeardTime没收到任何消息，则启动链路测试
		dwIndex = WSAWaitForMultipleEvents(1,&Event,FALSE,2000,FALSE);
		dwIndex = dwIndex - WAIT_OBJECT_0;
		if (dwIndex==WSA_WAIT_TIMEOUT||dwIndex==WSA_WAIT_FAILED)
		{
			waitTime+=2000;
			if (waitTime>3*2000)//3*m_HeardTime的时间没有接收到任何消息,则关闭会话
			{
				str.Format(L"%d 断开：超时",pnode->nThreadID);
				g_PaySerDlg->AddString(str);
				break;
			}
			else
				continue;
		}
		else
			waitTime = 0;

		// 分析什么网络事件产生
		Ret = WSAEnumNetworkEvents(sClient,Event,&NetWorkEvent);
		//其他情况
		if(!NetWorkEvent.lNetworkEvents)
		{
			continue;
		}
		else if (NetWorkEvent.lNetworkEvents & FD_READ)
		{
			NumberOfBytesRecvd=0;
			Ret = WSARecv(sClient,&Buffers,dwBufferCount,&NumberOfBytesRecvd,&Flags,NULL,NULL);
			if (Ret == SOCKET_ERROR)
			{
				revLen = 0;
				Buffers.len = 5;
				memset(szRequest,0,maxlen);
				g_PaySerDlg->AddString(L"SOCKET_ERROR in,continue...");
				continue;
			}
			else
			{
				revLen+=NumberOfBytesRecvd;

				byte start = szRequest[0];
				if (start != MSG_BEGN)
				{
					if (bOnePack)
					{
						str.Format(L"%d 断开：报文头错误,revLen=%d NumberOfBytesRecvd=%d",pnode->nThreadID,revLen,NumberOfBytesRecvd);
						g_PaySerDlg->AddString(str);
					}
					break;
				}
				else if (revLen<5)//报文头还没接收完
				{	
					Buffers.buf+=NumberOfBytesRecvd;
					Buffers.len = 5-revLen;//数据+0x03
					continue;
				}
				else
				{
					if (!bOnePack)
					{
						WaitForSingleObject(g_PaySerDlg->m_mutex, INFINITE);
						g_PaySerDlg->m_vNode.push_back(pnode);
						ReleaseMutex(g_PaySerDlg->m_mutex); 

						CString str;
						str.Format(L"%d 连入",pnode->nThreadID);
						g_PaySerDlg->AddString(str);

						str.Format(L"当前连接数：%d",g_PaySerDlg->m_vNode.size());
						g_PaySerDlg->m_NowSta.SetWindowTextW(str);
						bOnePack = true;
					}

					char LenBuff[4] = {0};
					memcpy(LenBuff,szRequest+1,4);

					//大端机，进行字节序转换
					if (g_Globle.EndianJudge())
						dataLen = ZZ_SWAP_4BYTE(*(long*)LenBuff);
					else
						dataLen = *(long*)LenBuff;

					if (dataLen+6>maxlen)
					{
						str.Format(L"%d 断开：接收报文长度：%d,已超出最大接收长度",pnode->nThreadID,dataLen);
						g_PaySerDlg->AddString(str);
						break;
					}

					if (revLen < 5+dataLen+1)
					{
						Buffers.buf+=NumberOfBytesRecvd;
						Buffers.len = dataLen+1-(revLen-5);//数据+0x03
						continue;
					}
					else if (revLen == 5+dataLen+1)
					{
						Buffers.buf=Buffers.buf-revLen+NumberOfBytesRecvd;
						Buffers.len=5;
						WSAResetEvent(Event);
						if (szRequest[revLen-1] != MSG_END)
						{
							str.Format(L"%d 断开：报文尾错误 byte：%d",pnode->nThreadID,szRequest[revLen-1]);
							g_PaySerDlg->AddString(str);
							break;
						}
					}
					else
					{
						str.Format(L"%d 断开：实际接收数据长度：%d 大于定义长度：%d",pnode->nThreadID,revLen,5+dataLen+1);
						g_PaySerDlg->AddString(str);
						break;
					}
				}
			}
			//此处进行消息处理,根据长度获取到数据buffer
			Json::Value js;
			char* data=new char[dataLen+1];
			memset(data,0,dataLen+1);
			memcpy(data,szRequest+5,dataLen);

			revLen = 0;
			Buffers.len = 5;
			memset(szRequest,0,maxlen);

			g_PaySerDlg->DoRun(data,js,pnode);
			g_PaySerDlg->SendTo(sClient,js);
			delete[] data;
		}
		else if(NetWorkEvent.lNetworkEvents & FD_CLOSE)
		{
			if (NetWorkEvent.iErrorCode[FD_CLOSE_BIT] != 0)
				continue;
			else
			{
				str.Format(L"%d 关闭连接",pnode->nThreadID);
				g_PaySerDlg->AddString(str);
				break;//退出
			}
		}
	}//while

	if (szRequest)
	{
		delete[] szRequest;
	}

	//删除节点,关闭socket
	closesocket(pnode->s);
	g_PaySerDlg->DeleteClient(pnode);
	WSACloseEvent(Event);
	return 0;
}


void CPayServerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	KillTimer(1);
	CloseHandle(m_mutex);
	m_bExit =TRUE;
	if (m_SocketHandle!=INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_SocketHandle,INFINITE);
		m_SocketHandle = INVALID_HANDLE_VALUE;
	}
}


void CPayServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CString str;
	USES_CONVERSION;
	m_listCtrl.DeleteAllItems();
	WaitForSingleObject(m_mutex, INFINITE); 
	for (int i=0;i<m_vNode.size();i++)
	{
		m_listCtrl.InsertItem(i,m_vNode[i]->strUser);
		str.Format(L"%d",m_vNode[i]->Addr.sin_port);
		m_listCtrl.SetItemText(i,1,str);
		m_listCtrl.SetItemText(i,2,m_vNode[i]->strName);
		m_listCtrl.SetItemText(i,3,m_vNode[i]->strLastHeartTime);
	}
	ReleaseMutex(m_mutex); 
	CDialogEx::OnTimer(nIDEvent);
}


void CPayServerDlg::OnBnClickedBtnHide()
{
	Hide();
}

//开机启动
void CPayServerDlg::OnBnClickedCkStartrun()
{
	HKEY hKey;
	CString strRegPath = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");

	//打开启动项
	if (RegOpenKeyEx(HKEY_CURRENT_USER, strRegPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) 
	{
		if (m_ckRun.GetCheck())
		{
			TCHAR szModule[MAX_PATH];
			GetModuleFileName(NULL, szModule, MAX_PATH);//得到本程序自身的全路径  
			//添加一个子Key,并设置值 
			RegSetValueEx(hKey, _T("PayServer"), 0, REG_SZ, (LPBYTE)szModule, (lstrlen(szModule) + 1)*sizeof(TCHAR));
		}
		else
		{
			RegDeleteValue(hKey, _T("PayServer"));
		}
		RegCloseKey(hKey); 
	}
	else
	{
		AfxMessageBox(_T("打开启动项失败"));
	}
}
