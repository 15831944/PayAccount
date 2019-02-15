#pragma once
#include "afxwin.h"
#include "PayAccountDlg.h"
#include "afxcmn.h"
#include "MyListCtrl.h"
#include "afxdtctl.h"
#include "..\common\EditSet.h"

// CDayCheckDlg 对话框
//日核算
#define  WM_DAYCHECK_CALL WM_USER+502

class CDayCheckDlg : public CPayAccountDlg
{
	DECLARE_DYNAMIC(CDayCheckDlg)

public:
	CDayCheckDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDayCheckDlg();

// 对话框数据
	enum { IDD = IDD_DPAY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnCallBack(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
public:
	CString m_LastStaffID;
	vector<STAFF_STU> m_vet;
	vector<DAYPAY> m_vCal;
	vector<PROJECT_STU> m_vProjects;
	vector<BOOK_STU> m_vBooks;
	vector<DAYPAY> m_vSaves;//保存时调用
	CMyListCtrl m_ListCtrl;
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
	void SetPerDayCtrlShow(int nCmdShow);
	void SetJiJianCtrlShow(int nCmdShow);
	void SetAllPayCtrl(DAYPAY_TYPE type,double money);
	void SetListCtrlValue();
	virtual void UpdateDlg();
	void SetComboStaffValue();

	CEditSet m_edit_per;
	CEditSet m_edit_day;
	CStatic m_GroupCtrl;
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnMcnSelchangeTimectrl(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox m_comboStaff;
	afx_msg void OnCbnSelchangeComboUser();
	afx_msg void OnEnChangeEditPer();
	afx_msg void OnEnChangeEditDay();
	CDateTimeCtrl m_DateTCtrl;
	afx_msg void OnDtnDatetimechangeDatetimepicker1(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic m_staticAll;
	CFont m_font;
	afx_msg void OnBnClickedBtnUpdate();

	//获取简单图书信息
	void SendToGetBook();
	void GetBook(Json::Value root);
	void SendToGetProject();
	void GetProject(Json::Value root);
	//获取简单职工信息
	void SendToGetStaff();
	void GetStaff(Json::Value root);
	void SendToGetDayPay();
	void GetDayPay(Json::Value root);
	void SendToDelDayPay(CString strStaffID,CString strDate);
	void SendToSaveDayPay();
	void SendToGetOnePay(CString strStaffID,int proID,CString strBookID,int nItem);
	void GetOnePay(Json::Value root);
	CStatic m_ts;
};
