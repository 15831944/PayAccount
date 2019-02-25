#include "stdafx.h"
#include "..\common\globle.h"
#include <iostream>
#include <fstream>
#include <shlobj.h>
#include <atlconv.h>

CGloble g_Globle;

CString TypeName[] = {  L"普通用户",  L"管理员",  L"超级管理员" };
CString BookType[] = { L"全开", L"对开"};
CString StaffType[] = {  L"工人",  L"师傅" ,  L"师傅+工人"};
CString ZyType[] = { L"全开四折页", L"四折页", L"大三折页", L"三折页", L"二折页"};
CString rkType[] = {L"未入",L"已入"};
CString proNumType[] = {L"印数",L"令数"};
CString proStaffType[] = {L"是",L"否"};
CString DateName[] = {L"全部",L"最近一周", L"最近一个月", L"最近三个月", L"半年内", L"一年内"};

CGloble::CGloble()
{
	CString workPath = GetWorkDir();
	m_ConfigFilePath = workPath+L"/config/config.ini";
}

CString CGloble::GetWorkDir() 
{  
	TCHAR pFileName[MAX_PATH]; 
	int nPos = GetCurrentDirectory( MAX_PATH, pFileName); 

	CString csFullPath(pFileName);  
	if( nPos < 0 ) 
		return CString(""); 
	else 
		return csFullPath; 
}

void CGloble::Convert(char* strIn,char* strOut, int sourceCodepage, int targetCodepage)
{
	int         len         = lstrlenA(strIn);
	int         unicodeLen  = MultiByteToWideChar(sourceCodepage, 0, strIn, -1, NULL, 0);
	wchar_t     pUnicode[1024] = {0};
	MultiByteToWideChar(sourceCodepage, 0, strIn, - 1, (LPWSTR)pUnicode, unicodeLen);

	BYTE        pTargetData[2048]   = {0};
	int         targetLen           = WideCharToMultiByte(targetCodepage, 0, (LPWSTR)pUnicode, -1, (char*)pTargetData,0, NULL, NULL);
	WideCharToMultiByte(targetCodepage, 0, (LPWSTR)pUnicode, -1,(char*)pTargetData, targetLen, NULL, NULL);
	lstrcpyA(strOut,(char*)pTargetData);
}

char* CGloble::EncodeToUTF8(const char* mbcsStr)  
{  
	wchar_t*  wideStr;  
	char*   utf8Str;  
	int   charLen;  

	charLen = MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, NULL, 0);  
	wideStr = (wchar_t*) malloc(sizeof(wchar_t)*charLen);  
	MultiByteToWideChar(CP_ACP, 0, mbcsStr, -1, wideStr, charLen);  

	charLen = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);  

	utf8Str = (char*) malloc(charLen);  

	WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, utf8Str, charLen, NULL, NULL);  

	free(wideStr);  
	return utf8Str;  

}  
char* CGloble::UTF8ToEncode(const char* mbcsStr)  
{  
	wchar_t*  wideStr;  
	char*   unicodeStr;  
	int   charLen;  

	charLen = MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, NULL, 0);    
	wideStr = (wchar_t*) malloc(sizeof(wchar_t)*charLen);  
	MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, wideStr, charLen);   

	charLen =WideCharToMultiByte(CP_ACP, 0, wideStr, -1, NULL, 0, NULL, NULL);   
	unicodeStr = (char*)malloc(charLen);  
	WideCharToMultiByte(CP_ACP, 0, wideStr, -1, unicodeStr, charLen, NULL, NULL);   

	free(wideStr);  
	return unicodeStr;  
}

bool CGloble::InitGloble()
{
	USES_CONVERSION;
	
	fstream _file;
	_file.open(m_ConfigFilePath,ios::in);
	if(!_file)
	{
		CString str;
		str.Format(L"找不到配置文件: %s ！",m_ConfigFilePath);
		MessageBox(NULL,str,L"error",NULL);
		return false;
	}
	else
	{
		TCHAR ip[30];
		GetPrivateProfileString(L"SOCKET",L"ip",L"",ip,30,m_ConfigFilePath);
		m_LocalIP=W2A(ip);	
		m_TcpPort=GetPrivateProfileInt(L"SOCKET",L"port",0,m_ConfigFilePath);
		TCHAR name[125];
		GetPrivateProfileString(L"SOCKET",L"name",L"",name,125,m_ConfigFilePath);
		m_strName=W2A(name);
	}
	_file.close();
	return true;
}

bool CGloble::get_local_ip()
{
	char hostname[128];
	int ret = gethostname(hostname, sizeof(hostname));
	if (ret == -1)
	{
		return false;
	}
	struct hostent *hent;
	hent = gethostbyname(hostname);
	if (NULL == hent)
	{
		return false;
	}
	while(*(hent->h_addr_list) != NULL)        //输出ipv4地址
	{
		m_LocalIP = inet_ntoa(*(struct in_addr *) *hent->h_addr_list);
		hent->h_addr_list++;
	}
	return true;
}

bool CGloble::InitSocket()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2,2),&wsadata)==0)
	{
		return true;
	}
	return false;
}

CString CGloble::ReturnBeginTime(EM_DATE_TYPE type)
{
	CTime tm; 
	tm=CTime::GetCurrentTime(); 
	int nyear,nmonth,nday;
	nyear = tm.GetYear();
	nmonth = tm.GetMonth();
	nday = tm.GetDay();

	int year=0,month=0,day=0;
	switch (type)
	{
	case EM_DATE_TYPE_1WEEK:
		{
			if (nday<=7)
			{
				if (nmonth<=1)
				{
					month = 12;
					year = nyear-1;
					int days = g_Globle.GetDays(year,month);
					day = days+nday-7;
				}
				else
				{
					month = nmonth-1;
					year = nyear;
					day = g_Globle.GetDays(year,month);
				}
			}
			else
			{
				day = nday-7;
				month = nmonth;
				year = nyear;
			}
		}
		break;
	case EM_DATE_TYPE_1MONTH:
		{
			if(nmonth <= 1)
			{
				month = 12;
				year = nyear-1;
			}
			else
			{
				year = nyear;
				month = nmonth-1;
			}
		}
		break;
	case EM_DATE_TYPE_3MONTH:
		{
			if(nmonth <= 3)
			{
				month = 12+nmonth-3;
				year = nyear-1;
			}
			else
			{
				year = nyear;
				month = nmonth-1;
			}
		}
		break;
	case EM_DATE_TYPE_6MONTH:
		{
			if(nmonth <= 6)
			{
				month = 12+nmonth-6;
				year = nyear-1;
			}
			else
			{
				year = nyear;
				month = nmonth-1;
			}
		}
		break;
	case EM_DATE_TYPE_12MONTH:
		{
			year = nyear - 1;
			month = nmonth;
		}
		break;
	}
	if (type != EM_DATE_TYPE_1WEEK)
	{
		int days = g_Globle.GetDays(year,month);
		if(days >= nday)
			day = nday;
		else
			day = days;
	}

	CString strBeginTime;
	strBeginTime.Format(L"%d/%02d/%02d",year,month,day);
	return strBeginTime;
}

int CGloble::GetDays(int year,int month)
{
	if (month == 2)
	{
		if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
			return 29;
		else 
			return 28;
	}
	switch (month)
	{
	case 1:return 31;
	case 3:return 31;
	case 4:return 30;
	case 5:return 31;
	case 6:return 30;
	case 7:return 31;
	case 8:return 31;
	case 9:return 30;
	case 10:return 31;
	case 11:return 30;
	case 12:return 31;
	}
	return 0;
}

bool CGloble::EndianJudge()
{
	short x;
	char x0;
	x=0x1122;
	x0=((char*)&x)[0];
	if(x0==0x11)
	{
		//大端
		return true;
	}
	else if(x0==0x22)
	{
		//小端
		return false;
	}
	return false;
}

int CGloble::GetFirstAsciiValue(CString strName)
{
	CString strPinyin = HzToPinYin(strName);
	char a = strPinyin[0];
	int value = a;
	return value;
}

bool In(wchar_t start, wchar_t end, wchar_t code)
{
	if   (code   >=   start   &&   code   <=   end)
	{
		return   true;
	}
	return   false;
}

char CGloble::convert(wchar_t n)
{
	if   (In(0xB0A1,0xB0C4,n))   return   'a';
	if   (In(0XB0C5,0XB2C0,n))   return   'b';
	if   (In(0xB2C1,0xB4ED,n))   return   'c';
	if   (In(0xB4EE,0xB6E9,n))   return   'd';
	if   (In(0xB6EA,0xB7A1,n))   return   'e';
	if   (In(0xB7A2,0xB8c0,n))   return   'f';
	if   (In(0xB8C1,0xB9FD,n))   return   'g';
	if   (In(0xB9FE,0xBBF6,n))   return   'h';
	if   (In(0xBBF7,0xBFA5,n))   return   'j';
	if   (In(0xBFA6,0xC0AB,n))   return   'k';
	if   (In(0xC0AC,0xC2E7,n))   return   'l';
	if   (In(0xC2E8,0xC4C2,n))   return   'm';
	if   (In(0xC4C3,0xC5B5,n))   return   'n';
	if   (In(0xC5B6,0xC5BD,n))   return   'o';
	if   (In(0xC5BE,0xC6D9,n))   return   'p';
	if   (In(0xC6DA,0xC8BA,n))   return   'q';
	if   (In(0xC8BB,0xC8F5,n))   return   'r';
	if   (In(0xC8F6,0xCBF0,n))   return   's';
	if   (In(0xCBFA,0xCDD9,n))   return   't';
	if   (In(0xCDDA,0xCEF3,n))   return   'w';
	if   (In(0xCEF4,0xD188,n))   return   'x';
	if   (In(0xD1B9,0xD4D0,n))   return   'y';
	if   (In(0xD4D1,0xD7F9,n))   return   'z';
	return   '/0';
}

CString CGloble::HzToPinYin(CString str)
{
	CString PinYin;
	//std::string   sChinese   =   “张三丰”;   //   输入的字符串
#ifdef UNICODE
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, str.GetBuffer(0), -1, NULL, 0, NULL, FALSE);
	char *psText;
	psText = new char[dwNum];
	if (!psText)
		delete []psText;
	WideCharToMultiByte(CP_OEMCP, NULL, str.GetBuffer(0), -1, psText, dwNum, NULL, FALSE);
#endif
	std::string   sChinese=psText;
	char c;
	char   chr[3];
	wchar_t   wchr=0;
	int len=sChinese.length()/2;
	char*   buff   =   new   char[len];
	memset(buff,0,sizeof(char)*sChinese.length()/2+1);

	for   (int i=0,j=0;i<(sChinese.length()/2);++i)
	{
		memset(chr,0,sizeof(chr));
		chr[0]   =   sChinese[j++];
		chr[1]   =   sChinese[j++];
		chr[2]   =   '\0';

		//   单个字符的编码   如：'我'   =   0xced2
		wchr   =   0;
		wchr   =   (chr[0]   &   0xff)   <<   8;
		wchr   |=   (chr[1]   &   0xff);

		buff[i]   =   convert(wchr);
	}
	PinYin=buff;
	return  PinYin;
}

char* CGloble::CombineSendData(string strData,long& allLen)
{
	long dataLen = strData.length();
	allLen = dataLen + 6;
	char* sendBuf = NULL;
	sendBuf= new char[allLen];
	memset(sendBuf,0,allLen);
	//消息头标志
	sendBuf[0] = MSG_BEGN;
	//数据长度
	char clen[4]={0};
	long zzDataLen=dataLen;
	if (EndianJudge())
	{
		zzDataLen=ZZ_SWAP_4BYTE(zzDataLen);
	}
	memcpy(clen,(char*)&zzDataLen,4);
	memcpy(sendBuf+1,clen,4);
	//数据
	memcpy(sendBuf+5,strData.c_str(),dataLen);
	//消息尾标志
	sendBuf[dataLen+5] = MSG_END;
	return sendBuf;
}