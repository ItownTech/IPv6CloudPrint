// Collector.cpp: implementation of the CCollector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Collector.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCollector::CCollector()
{

}

CCollector::~CCollector()
{

}

//////////////////////////////////////////////////////////////////////////
// ����ҳ��ַ��������ҳԴ���ַ��� bEncode(TRUE:UTF-8, FALSE:ANSI )    //
//////////////////////////////////////////////////////////////////////////
CString CCollector::Open(char * cpUrl, BOOL bEncode)
{
	ASSERT(cpUrl);

	CString strData = "";
	int nRead = 0;
	try
	{
		CInternetSession session;
		CHttpFile *pF=(CHttpFile *)session.OpenURL(cpUrl);
		CString strTempData = "";

		while (pF->ReadString(strTempData)) 
		{
			strData+="\r\n";
			strData+=strTempData;
		}

		if (bEncode)
		{
			ToUft8(strData, CP_UTF8, CP_ACP);
		}

		delete pF;
		session.Close();
	}
	catch (CException *e) {
		TCHAR   szError[1024];   
		e->GetErrorMessage(szError,1024);

		strData.Format("%sCCollector::Open ʧ�ܣ�URL=%s\n", DS_ERROR_FLAG, cpUrl);
	}
	return strData;
}

BOOL CCollector::IsError(CString strStr)
{
	string str = strStr.GetBuffer(0);

	if(strstr(str.c_str(), DS_ERROR_FLAG) == NULL)
	{
		return FALSE;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// ���ַ������� ANSI UTF-8 ��ת                                         //
// CP_ACP=ANSI,CP_UTF8=utf-8                                            //
//////////////////////////////////////////////////////////////////////////
void CCollector::ToUft8(CString &str,int sourceCodepage,int targetCodepage)
{
	int len=str.GetLength(); 
	
	int unicodeLen=MultiByteToWideChar(sourceCodepage,0,str,-1,NULL,0); 
	
	wchar_t * pUnicode; 
	pUnicode=new wchar_t[unicodeLen+1]; 
	
	memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t)); 
	
	MultiByteToWideChar(sourceCodepage,0,str,-1,(LPWSTR)pUnicode,unicodeLen); 
	
	BYTE * pTargetData; 
	int targetLen=WideCharToMultiByte(targetCodepage,0,(LPWSTR)pUnicode,-1,(char *)pTargetData,0,NULL,NULL); 
	
	pTargetData=new BYTE[targetLen+1]; 
	memset(pTargetData,0,targetLen+1); 
	
	WideCharToMultiByte(targetCodepage,0,(LPWSTR)pUnicode,-1,(char *)pTargetData,targetLen,NULL,NULL); 
	
	str.Format("%s",pTargetData);
	
	delete pUnicode; 
	delete pTargetData; 
}

/*******************************************************
* ��ô�pos��ʼ����ʼ��ǩΪcpStartStr��������ǩΪcpEndStr���ַ���
******************************************************/
int CCollector::GetKey(CString strSource, char *cpStartStr, char * cpEndStr, int iStartPos, CString &strValue)
{
	strValue = "";
	int iPos = strSource.Find(cpStartStr, iStartPos);
	if (iPos == -1)
	{
		return -1;
	}

	iPos += strlen(cpStartStr);

	int iEPos = strSource.Find(cpEndStr, iPos);
	if (iEPos == -1)
	{
		return -1;
	}
	
	int iCount = iEPos - iPos;

	if (iCount>0)
	{
		string str = strSource.GetBuffer(0);
		strValue.Format("%s", str.substr(iPos, iCount).c_str());
	}

	return iEPos+strlen(cpEndStr);
}

/*******************************************************
* �ָ��ַ��� 
* lpszStr Դ�ַ�����lpszSplit �ָ��ַ�����bAllowNullString �ָ�ʧ�ܺ��Ƿ񷵻�Դ�ַ���
* rArrString �ָ����ַ�������
* �ַ������鳤�� �ɹ�
******************************************************/
int CCollector::SplitString(LPCTSTR lpszStr, LPCTSTR lpszSplit, CStringArray& rArrString, BOOL bAllowNullString)   
{   
	rArrString.RemoveAll();
	CString szStr = lpszStr;
	szStr.TrimLeft();
	szStr.TrimRight();
	if(szStr.GetLength()==0)   
	{
		return 0;
	}   
	CString szSplit = lpszSplit;
	if(szSplit.GetLength() == 0)
	{   
		rArrString.Add(szStr);
		return 1;
	}
	CString s;   
	int n;
	do {   
		n = szStr.Find(szSplit);
		if(n > 0)
		{   
			rArrString.Add(szStr.Left(n));   
			szStr = szStr.Right(szStr.GetLength()-n-szSplit.GetLength());
			szStr.TrimLeft();   
		}
		else if(n==0)   
		{   
			if(bAllowNullString)   
				rArrString.Add(_T(""));   
			szStr = szStr.Right(szStr.GetLength()-szSplit.GetLength());   
			szStr.TrimLeft();   
		}   
		else
		{   
			if((szStr.GetLength()>0) || bAllowNullString)   
				rArrString.Add(szStr);
			break;   
		}
	} while(1);
	return rArrString.GetSize();   
}

int CCollector::GetSafeStr(char * cpDest, char * cpSource, int iSize, int iMax)
{
	int nRet = -1;
	try
	{
		if(iSize < iMax-1)
		{
			strcpy(cpDest, cpSource);
			return 0;
		}

		for(int i=0; i<iMax-1; i++)
		{
			if(i<iMax-2)
			{
				if((byte)cpSource[i]>0x7F)
				{
					cpDest[i] = cpSource[i];
					i++;
				}

				cpDest[i] = cpSource[i];
			}
			else
			{
				cpDest[i] = '\0';
			}
		}
		nRet = 0;
	}
	catch (...) {
		nRet = -1;
	}
	return nRet;
}

//////////////////////////////////////////////////////////////////////////
//�ַ���ʱ��ת���� time_t ʱ��                                          //
//////////////////////////////////////////////////////////////////////////
time_t CCollector::ChangeTime(CString cpTime)
{
	struct tm tm1; 
	memset(&tm1,0,sizeof(tm));
	time_t   tt1; 
	
	sscanf(cpTime,"%d-%d-%d %d:%d", &tm1.tm_year, &tm1.tm_mon, &tm1.tm_mday,&tm1.tm_hour,&tm1.tm_min);
	tm1.tm_sec=0;
	tm1.tm_year -= 1900;
	tm1.tm_mon -= 1;
	tt1 = mktime(&tm1);
	return tt1;
}

// �ж��ַ����Ƿ�UTF8����
BOOL CCollector::IsUTF8String(char * str, ULONGLONG length)
{
	int i = 0;
	DWORD nBytes = 0; // UFT8����1-6���ֽڱ���,ASCII��һ���ֽ�
	UCHAR chr = 0x0;
	BOOL bAllAscii = TRUE; // ���ȫ������ASCII, ˵������UTF-8
	for(i = 0; i < length; i ++)
	{
		chr = *(str + i);
		if( (chr & 0x80) != 0 ) // �ж��Ƿ�ASCII����,�������,˵���п�����UTF-8,ASCII��7λ����,����һ���ֽڴ�,���λ���Ϊ0,o0xxxxxxx
			bAllAscii = FALSE;
		if(nBytes == 0) //�������ASCII��,Ӧ���Ƕ��ֽڷ�,�����ֽ���
		{
			if(chr >= 0x80)
			{
				if(chr >= 0xFC && chr <= 0xFD)
					nBytes = 6;
				else if(chr >= 0xF8)
					nBytes = 5;
				else if(chr >= 0xF0)
					nBytes = 4;
				else if(chr >= 0xE0)
					nBytes = 3;
				else if(chr >= 0xC0)
					nBytes = 2;
				else
				{
					return FALSE;
				}
				nBytes--;
			}
		}
		else //���ֽڷ��ķ����ֽ�,ӦΪ 10xxxxxx
		{
			if( (chr&0xC0) != 0x80 )
			{
				return FALSE;
			}
			nBytes--;
		}
	}
	if( nBytes > 0 ) //Υ������
	{
		return FALSE;
	}
	if( bAllAscii ) //���ȫ������ASCII, ˵������UTF-8
	{
		return FALSE;
	}
	return TRUE;
}

std::string CCollector::UrlEncode(const std::string& szToEncode)
{
	std::string src = szToEncode;
	char hex[] = "0123456789ABCDEF";
	string dst;
	
	for (size_t i = 0; i < src.size(); ++i)
	{
		unsigned char cc = src[i];
		if (isascii(cc))
		{
			if (cc == ' ')
			{
				dst += "%20";
			}
			else
				dst += cc;
		}
		else
		{
			unsigned char c = static_cast<unsigned char>(src[i]);
			dst += '%';
			dst += hex[c / 16];
			dst += hex[c % 16];
		}
	}
	return dst;
}

std::string CCollector::UrlDecode(const std::string& szToDecode)
{
	std::string result;
	int hex = 0;
	for (size_t i = 0; i < szToDecode.length(); ++i)
	{
		switch (szToDecode[i])
		{
		case '+':
			result += ' ';
			break;
		case '%':
			if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
			{
				std::string hexStr = szToDecode.substr(i + 1, 2);
				hex = strtol(hexStr.c_str(), 0, 16);
				//��ĸ������[0-9a-zA-Z]��һЩ�������[$-_.+!*'(),] ���Լ�ĳЩ������[$&+,/:;=?@]
				//���Բ���������ֱ������URL
				if (!((hex >= 48 && hex <= 57) ||	//0-9
					(hex >=97 && hex <= 122) ||	//a-z
					(hex >=65 && hex <= 90) ||	//A-Z
					//һЩ������ż�������[$-_.+!*'(),]  [$&+,/:;=?@]
					hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29
					|| hex == 0x2a || hex == 0x2b|| hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f
					|| hex == 0x3A || hex == 0x3B|| hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f
					))
				{
					result += char(hex);
					i += 2;
				}
				else result += '%';
			}else {
				result += '%';
			}
			break;
		default:
			result += szToDecode[i];
			break;
		}
	}
	return result;
}