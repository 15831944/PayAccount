#pragma once
#include "..\common\globle.h"
#include "sqlite3.h"

class CDbData
{
public:
	CDbData();
	~CDbData();

public:
	sqlite3 *m_sqlite;
	CString m_strSql;
public:
	bool InitSqlite3();
	//��ʼ�����ݿ�
	bool InitData();
	//��ȡ��ǰʱ��
	CString GetTimeNow();

	//ְ��
	bool AddStaff(CString strName,CString strSex,int age,CString strStaffID, CString idcard,CString strTel,int type);
	bool ModifyStaff(CString strName,CString strSex,int age,CString strStaffID,CString idcard,CString strTel,int type);
	bool DelStaff(CString strStaffID);
	bool _JudgeStaff(CString idcard, Json::Value& root);
	vector<STAFF_STU> GetStaff(CString strName);
	bool GetStaffs(CString strKeyWord,Json::Value& root,int nstart = 0,int number = 20);
	bool GetSampleStaffs(Json::Value& root);
	//��ȡְ������
	bool GetStaffType(CString strStaffID, int& type);

	//ͼ��
	bool AddBook(CString strID, CString strName,CString strCbs,CString strDate,int nBc1,int nBc2, int nSize1,int nSize2,int nKb, double yz,int ys,int bc,double ls,int yzType,int zyType,int rkType,CString strMsg);
	bool DelBook(CString strBookID);
	bool ModifyBook(CString strID, CString strName,CString strCbs,CString strDate,int nBc1,int nBc2, int nSize1,int nSize2,int nKb, double yz,int ys,int bc,double ls,int yzType,int zyType,int rkType,CString strMsg);
	vector<BOOK_STU> GetBook(CString strKeyWord,int rkType,int& nCount,int nStart = 0,int nNum = 20);
	bool GetBooks(Json::Value& root,CString strKeyWord,int rkType,EM_DATE_TYPE dateType,int nStart = 0,int nNum = 20);
	bool GetSampleBooks(Json::Value& root,BOOK_RK rkType);
	BOOK_STU GetBookFromID(CString strBookID);
	bool JudgeBook(CString strName,bool& bExit);
	bool _JudgeBook(CString strName, Json::Value& root);
	//ͨ��bookID��ȡӡ��,ӡ����ӡ������
	bool GetYzAndType(CString strBookID,double& yz,int& ys,int& type);
	//ͨ��bookID��ȡ��ҳ����
	bool GetZyType(CString strBookID,int& zyType);

	//�˺�
	vector<USER_STU> GetUser();
	bool GetUsers(Json::Value& root);
	bool AddUser(CString strName,CString strPass,int nType);
	bool DelUser(CString strName);
	bool JudgeUser(CString strName, bool& bExit);
	bool _JudgeUser(CString strName, Json::Value& root);
	bool bLogin(CString strName,CString strPass,int show_pass,bool& bOk,int& type);
	bool Login(CString strName,CString strPass,int show_pass,Json::Value& root);
	bool SetLastLoginName(CString strName);
	bool GetLastLoginName(CString& strName);
	bool CheckPwd(CString strName,CString strPwd);
	bool ModifyPwd(CString strName, CString strPwd);

	//��Ŀ
	vector<PROJECT_STU> GetProjectList(PRO_STAFF_TYPE type = PRO_STAFF_TYPE_MAX);
	bool GetProjectLists(Json::Value& root,PRO_STAFF_TYPE type = PRO_STAFF_TYPE_MAX);
	//�����Ŀ
	bool AddProject(CString strProName,PRO_NUM_TYPE pn_type,PRO_STAFF_TYPE ps_type);
	//�޸�
	bool ModifyProject(int id,CString strProName,PRO_NUM_TYPE pn_type,PRO_STAFF_TYPE ps_type);
	//ɾ����Ŀ
	bool DelProject(CString strIDs);
	bool DelOneProject(int id);
	//������Ŀ����˳��
	bool SaveProNdex(Json::Value root);
	//ͨ����Ŀ�����жϸ���Ŀ�Ƿ��Ѿ�����
	bool JudgePro(CString strName,bool& bExit);
	bool _JudgePro(CString strName, Json::Value& root);

	//����ͻ�ȡ������ҳ�����б�
	bool SaveDaiPay(STU_DAI_PAY stu);
	bool GetDaiPay(STU_DAI_PAY& stu);
	bool _GetDaiPay(Json::Value& root);
	//����ͻ�ȡ��ҳ�����б�
	bool SaveZheYePay(STU_ZHEYE_PAY stu);
	bool GetZheYePay(STU_ZHEYE_PAY& stu);
	bool _GetZheYePay(Json::Value& root);
	//����ͻ�ȡ��ҳ����
	bool SaveDianyePay(CString strdown,CString strup);
	bool GetDianyePay(CString& strdown,CString& strup);
	bool _GetDyPay(Json::Value& root);

	//����ͻ�ȡ������Ŀ�����б�
	bool SaveOtherPay(int proID, vector<OTHER_PRO_PAY> vec);
	vector<OTHER_PRO_PAY> GetOtherPay(int proID);
	bool _GetOtherPay(Json::Value& root,int proID);
	//ɾ��������Ŀ����
	bool DelOtherPay(CString strIDs);
	//ͨ�� ��������Ŀ��������ȡ����
	CString GetOnePay(CString strStaffID, int proID, CString strBookID);
	bool _GetOnePay(Json::Value& root,CString strStaffID, int proID, CString strBookID);
	//ͨ����ҳID��Ա�����ͻ�ȡ����
	CString GetZyPayFromID(int zyID,int staffType);
	//ͨ�� ��Ŀ��������ȡ������Ŀ����
	CString GetOtherPayFromID(int proID, CString strBookID);

	//�պ���
	bool DeleteDayPay(CString strStaffID,CString strDate=NULL);
	//ɾ��ָ������
	bool DeleteDayPay(int id);
	bool DeleteDayPay(CString strStaffID,int nProID,CString strBookID, CString strDate);
	bool AddDayPay(CString strStaffID, vector<DAYPAY> vec, CString strDate);
	vector<DAYPAY> GetDayPay(CString strStaffID, CString strDate);
	bool _GetDayPay(Json::Value& root,CString strStaffID, CString strDate);

	//�º���
	bool _GetMouthPay(Json::Value& js,Json::Value root);
	vector<DAYPAY> GetWorkCalMonth(CString strStaffID, CString strDate);

	//ͳ��
	bool SaveWorkCal(CString strStaffID, vector<WOCKCAL> vec, CString strDate);
	vector<WOCKCAL> GetWorkCal(CString strStaffID, CString strDate);
	bool DeleteWorkCal(CString strStaffID,CString strDate=NULL);
	bool DeleteWorkCal(CString strStaffID,int nProID,CString strBookID, CString strDate,CString strTime);
	vector<STU_DETAIL> GetDetail(int proID,CString strBookID);
	bool _GetDetails(Json::Value& root,vector<PROJECT_STU> vProIDs,CString strBookID);
	//����
	int GetProgress(CString strBookID,int proID);
	bool GetStaffWrite(Json::Value& js,CString strBookID,int proID);
	bool SaveStaffWrite(CString strBookID,int proID,double number);
	bool _GetProgress(Json::Value& js,Json::Value root, CString strBookID);
	bool DelProgressByBookID(CString strBookID);
	bool DelProgressByProID(int proID);
};