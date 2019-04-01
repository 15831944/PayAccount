
// PayServerDlg.h : 头文件
//

#pragma once
#include <string>
#include "afxwin.h"
#include "afxcmn.h"

using std::string;

#define WM_SOCKMSG   WM_USER+1
#define WM_SHOWTASK (WM_USER + 2)

enum SOCK_MSG
{
	SOCK_MSG_OK,
	SOCK_ERROR_CREATESOCK,
	SOCK_ERROR_BIND,
	SOCK_ERROR_CREATEEVENT,
	SOCK_MSG_CLOSE,
	SOCK_MSG_ONE_LINK,
	SOCK_MSG_ONE_CLOSE,
	SOCK_MSG_TIMEOUT,
	SOCK_MSG_MAX
};

typedef struct _NODE_ 
{
	SOCKET s;
	sockaddr_in Addr;
	int nThreadID;//线程ID
	HANDLE hHandle;//线程句柄
	CString strUser;//账号
	CString strName;//名称
	CString strLastHeartTime;//上次心跳时间
	CString strMsg;//描述

}Node,*pNode;

// CPayServerDlg 对话框
class CPayServerDlg : public CDialogEx
{
// 构造
public:
	CPayServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_PAYSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	LRESULT OnShowTask(WPARAM wParam, LPARAM lParam) ;

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnShow();
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnSockMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	bool AddClientList(SOCKET s,sockaddr_in addr);
	void DeleteClient(Node* pnode);
	void DoRun(string strData,Json::Value& js,Node* pnode);
	void SendTo(SOCKET sock,Json::Value js);
	void AddString(CString str);
	void InitListCtrl();
	void InitChechBox();
	void Hide();
public:
	NOTIFYICONDATA m_nid;
	HANDLE   m_SocketHandle;
	BOOL     m_bExit;
	vector<Node*>    m_vNode;
	afx_msg void OnDestroy();
	CListBox m_ListBox;
	CListCtrl m_listCtrl;
	CStatic m_staIP;
	HANDLE m_mutex;
	HANDLE m_sendMutex;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnHide();
	CButton m_ckRun;
	afx_msg void OnBnClickedCkStartrun();
	CStatic m_NowSta;
};
