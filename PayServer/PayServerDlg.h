
// PayServerDlg.h : ͷ�ļ�
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
	int nThreadID;//�߳�ID
	HANDLE hHandle;//�߳̾��
	CString strUser;//�˺�
	CString strName;//����
	CString strLastHeartTime;//�ϴ�����ʱ��
	CString strMsg;//����

}Node,*pNode;

// CPayServerDlg �Ի���
class CPayServerDlg : public CDialogEx
{
// ����
public:
	CPayServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PAYSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	LRESULT OnShowTask(WPARAM wParam, LPARAM lParam) ;

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
