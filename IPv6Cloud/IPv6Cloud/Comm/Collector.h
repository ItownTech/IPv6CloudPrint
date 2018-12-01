// Collector.h: interface for the CCollector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLLECTOR_H__8AFC38E9_E9EB_442E_AC2C_C9CA7F4E7030__INCLUDED_)
#define AFX_COLLECTOR_H__8AFC38E9_E9EB_442E_AC2C_C9CA7F4E7030__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxinet.h>
#include <string>
using namespace std;

#define     DS_ERROR_FLAG    "$_FEB_ERROR_$"

#define     DS_ERROR_LEN     256

class CCollector  
{
public:
	// ����ҳ��ַ��������ҳԴ���ַ��� bEncode(TRUE:UTF-8, FALSE:ANSI )
	CString Open(char * cpUrl, BOOL bEncode = TRUE);
	// ����ת��
	void ToUft8(CString &str,int sourceCodepage,int targetCodepage);
	// �Ƿ��쳣
	BOOL IsError(CString strStr);
	// ��ô�pos��ʼ����ʼ��ǩΪcpStartStr��������ǩΪcpEndStr���ַ���
	int GetKey(CString strSource, char *cpStartStr, char * cpEndStr, int iStartPos, CString &strValue);
	// �ָ��ַ���
	int SplitString(LPCTSTR lpszStr, LPCTSTR lpszSplit, CStringArray& rArrString, BOOL bAllowNullString);
	// ���ַ���cpDest�л����󳤶�ΪiMax�İ�ȫ�ַ���
	int GetSafeStr(char * cpDest, char * cpSource, int iSize, int iMax);
	// �ַ���ʱ��ת���� time_t ʱ��
	time_t ChangeTime(CString cpTime);
	// �ж��ַ����Ƿ�UTF8����
	BOOL IsUTF8String(char * str, ULONGLONG length);

	std::string UrlEncode(const std::string& szToEncode);
	std::string UrlDecode(const std::string& szToDecode);

public:
	CCollector();
	virtual ~CCollector();
};

#endif // !defined(AFX_COLLECTOR_H__8AFC38E9_E9EB_442E_AC2C_C9CA7F4E7030__INCLUDED_)
