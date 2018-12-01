#include "stdafx.h"
#include "HttpJsonReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static void OutputDebugPrintf(const char* strOutputString, ...)
{
	char strBuffer[4096] =
	{
		0
	};
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
	va_end(vlArgs);
	OutputDebugString(strBuffer);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHttpJsonReader::CHttpJsonReader(char* strHttpServerAddr)
{
	m_strHttpServerAddr = strHttpServerAddr;
	m_pSession = NULL;
	m_pHttpFile = NULL;
	m_strURL = NULL;
}

CHttpJsonReader::CHttpJsonReader(CHttpJsonReader* pOther)
{
	m_strHttpServerAddr = pOther->m_strHttpServerAddr;
	m_pSession = NULL;
	m_pHttpFile = NULL;
	m_strURL = NULL;
}

CHttpJsonReader::~CHttpJsonReader()
{
	CloseHttpFile();
	CloseSession();
}

char* CHttpJsonReader::GetHttpServerAddr()
{
	return m_strHttpServerAddr;
}

/***************************************************************************/
/* ��ָ�������ݽ���ΪJson����,��ȡ�����ɹ�����true                         */
/***************************************************************************/
bool CHttpJsonReader::ParseJson(const char* strJson, Json::Value& valueJson)
{
	bool bSucceed = false;
	if (strJson && strJson[0])
	{
		bSucceed = m_reader.parse(strJson, valueJson);
	}
	return bSucceed;
}

/***************************************************************************/
/* ��ָ�������URL��ַ��ȡ����,�������ݽ���ΪJson����,��ȡ�����ɹ�����true */
/***************************************************************************/
bool CHttpJsonReader::ReadJson(const char* strRelativeUrl, Json::Value& valueJson,
							   DWORD dwTimeout, bool* pbIsUTF8, char** ppstrValue)
{
	bool bSucceed = false;
	if (strRelativeUrl && strlen(strRelativeUrl) > 0)
	{
		char* strURL = new char[strlen(strRelativeUrl) + strlen(m_strHttpServerAddr) + 48];
		if (strRelativeUrl[0] == '/')
		{
			sprintf(strURL, "http://%s%s", m_strHttpServerAddr, strRelativeUrl);
		}
		else
		{
			sprintf(strURL, "http://%s/%s", m_strHttpServerAddr, strRelativeUrl);
		}


		int nTextLen = 0;
		std::string strText;
		if (pbIsUTF8)
		{
			CString csContentType;
			nTextLen = GetUrlContent(strURL, strText, true, dwTimeout, &csContentType);
			csContentType.MakeLower();
			int pos = csContentType.Find("utf-8");
			*pbIsUTF8 = pos > 0;
		}
		else
		{
			nTextLen = GetUrlContent(strURL, strText, true, dwTimeout);
		}
		if (nTextLen > 0)
		{
			// ppstrValue��Ϊ��ʱ,*ppstrValueֱ�ӷ����ַ���,���������json����
			if (ppstrValue)
			{
				*ppstrValue = new char[nTextLen + 1];
				if (*ppstrValue)
				{
					strcpy(*ppstrValue, strText.c_str());
					bSucceed = true;
				}
			}
			else
			{
				bSucceed = m_reader.parse(strText, valueJson, false);
			}
		}

		delete[] strURL;
	}
	return bSucceed;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool CHttpJsonReader::ReadJson(const char* strWebService, const char* strRelativeUrl, Json::Value& valueJson, DWORD dwTimeout /* = 0 */, bool* pbIsUTF8 /* = NULL */, char** ppstrValue /* = NULL */)
{
	if (strWebService == NULL)
	{
		return ReadJson(strRelativeUrl, valueJson, dwTimeout, pbIsUTF8, ppstrValue);
	}

	bool bSucceed = false;
	if (strRelativeUrl && strlen(strRelativeUrl) > 0)
	{
		char* strURL = new char[strlen(strRelativeUrl) + strlen(strWebService) + 48];
		if (strRelativeUrl[0] == '/')
		{
			sprintf(strURL, "http://%s%s", strWebService, strRelativeUrl);
		}
		else
		{
			sprintf(strURL, "http://%s/%s", strWebService, strRelativeUrl);
		}
		
		
		int nTextLen = 0;
		std::string strText;
		if (pbIsUTF8)
		{
			CString csContentType;
			nTextLen = GetUrlContent(strURL, strText, true, dwTimeout, &csContentType);
			csContentType.MakeLower();
			int pos = csContentType.Find("utf-8");
			*pbIsUTF8 = pos > 0;
		}
		else
		{
			nTextLen = GetUrlContent(strURL, strText, true, dwTimeout);
		}
		if (nTextLen > 0)
		{
			// ppstrValue��Ϊ��ʱ,*ppstrValueֱ�ӷ����ַ���,���������json����
			if (ppstrValue)
			{
				*ppstrValue = new char[nTextLen + 1];
				if (*ppstrValue)
				{
					strcpy(*ppstrValue, strText.c_str());
					bSucceed = true;
				}
			}
			else
			{
				bSucceed = m_reader.parse(strText, valueJson, false);
			}
		}
		
		delete[] strURL;
	}
	return bSucceed;
}

/*******************************************************************************/
/* ��ָ�������URL��ַPOST��ȡ����,�������ݽ���ΪJson����,��ȡ�����ɹ�����true */
/*******************************************************************************/
bool CHttpJsonReader::PostReadJson(const char* strWebService, const char* strRelativeUrl, Json::Value& valueJson, 
								   const char* strParam, bool* pbIsUTF8, char** ppstrValue)
{
	bool bSucceed = false;
	if (strRelativeUrl && strRelativeUrl[0] && strParam && strParam[0])
	{
		char* strURL = new char[strlen(strRelativeUrl) + strlen(strWebService) + 48];
		if (strRelativeUrl[0] == '/')
		{
			sprintf(strURL, "http://%s%s", strWebService, strRelativeUrl);
		}
		else
		{
			sprintf(strURL, "http://%s/%s", strWebService, strRelativeUrl);
		}
		

		int nTextLen = 0;
		std::string strText;
		if (pbIsUTF8)
		{
			CString csContentType;
			nTextLen = PostUrlContent(strURL, strText, strParam, &csContentType);
			csContentType.MakeLower();
			int pos = csContentType.Find("utf-8");
			*pbIsUTF8 = pos > 0;
		}
		else
		{
			nTextLen = PostUrlContent(strURL, strText, strParam);
		}

		if (nTextLen > 0)
		{
			//OutputDebugString("Json���أ�\n");
			//OutputDebugString(strText.c_str());
			// ppstrValue��Ϊ��ʱ,*ppstrValueֱ�ӷ����ַ���,���������json����
			if (ppstrValue)
			{
				*ppstrValue = new char[nTextLen + 1];
				if (*ppstrValue)
				{
					strcpy(*ppstrValue, strText.c_str());
					bSucceed = true;
				}
			}
			else
			{
				bSucceed = m_reader.parse(strText, valueJson, false);
				OutputDebugString("\n----------------------------------->\n");
				OutputDebugString(strText.c_str());
				OutputDebugString("\n<-----------------------------------\n");
			}
		}
		
		delete[] strURL;
	}
	return bSucceed;
}

void CHttpJsonReader::CloseHttpFile()
{
	if (m_pHttpFile)
	{
		m_pHttpFile->Close();
		delete m_pHttpFile;
		m_pHttpFile = NULL;
	}
}

void CHttpJsonReader::CloseSession()
{
	if (m_pSession)
	{
		m_pSession->Close();
		delete m_pSession;
		m_pSession = NULL;
	}
}
/************************************************************************/
/* ����HTTP���ӵ��߳�                                                   */
/************************************************************************/
DWORD CHttpJsonReader::OpenURLThread(LPVOID lpParam)
{
	DWORD dwRet = 1;
	CHttpJsonReader* pThis = (CHttpJsonReader*) lpParam;
	if (pThis->m_pSession && pThis->m_strURL)
	{
		try
		{
			pThis->m_pHttpFile = (CHttpFile*) pThis->m_pSession->OpenURL(pThis->m_strURL, 1, INTERNET_FLAG_TRANSFER_ASCII |INTERNET_FLAG_DONT_CACHE, NULL, 0);
			dwRet = 0;
		}
		catch (CInternetException* pException)
		{
			dwRet = 2;
			pThis->CloseHttpFile();
			pException->Delete();
		}
	}
	return dwRet;
}

/************************************************************************/
/* ȡ��URL������                                                        */
/************************************************************************/
int CHttpJsonReader::GetUrlContent(const char* strURL, std::string& strResult, bool bExceptBOM, 
								   DWORD dwTimeout, CString* pcsContentType)
{
	m_pSession = new CInternetSession(NULL, 0);
	m_pHttpFile = NULL;
	strResult.erase();
	m_strURL = strURL;
	
	try
	{
		/*
		CString cstrLine;
		httpFile = (CHttpFile*) session.OpenURL(strURL);
		if (httpFile != NULL)
		{
			// ReadString��������bug,�ڶ�ȡ��HTTP���ݳ�����127��������ʱ,ReadString����FALSE
			while (httpFile->ReadString(cstrLine))
			{
				strResult += cstrLine;
			}
			httpFile->Close();
			delete httpFile;
			httpFile = NULL;
		}
		*/

		if (dwTimeout)
		{
			/* ���ó�ʱ
			m_pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 5000);      // 5������ӳ�ʱ
			m_pSession->SetOption(INTERNET_OPTION_SEND_TIMEOUT, 1000);         // 1��ķ��ͳ�ʱ
			m_pSession->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 7000);      // 7��Ľ��ճ�ʱ
			m_pSession->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 1000);    // 1��ķ��ͳ�ʱ
			m_pSession->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 7000); // 7��Ľ��ճ�ʱ
			m_pSession->SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);         // 1������
			*/

			DWORD  dwThreadID = 0;
			HANDLE hThread = CreateThread(NULL, 0, OpenURLThread, this, 0, &dwThreadID);
			if (WaitForSingleObject(hThread, dwTimeout) == WAIT_TIMEOUT)
			{
				m_pSession->Close();
				WaitForSingleObject(hThread, INFINITE);

				char strInfo[320];
				sprintf(strInfo, "GetUrlContent��ʱ. strURL = %s", strURL);
				OutputDebugPrintf(strInfo);
			}
		}
		else
		{
			m_pHttpFile = (CHttpFile*) m_pSession->OpenURL(strURL, 1, INTERNET_FLAG_TRANSFER_ASCII |INTERNET_FLAG_DONT_CACHE, NULL, 0);
		}

		if (m_pHttpFile)
		{
			if (pcsContentType)
			{
				// Content-Type: text/html; charset=gbk
				m_pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_TYPE, *pcsContentType);
			}

			char strBuffer[4096] = {0};
			while (m_pHttpFile->ReadString(strBuffer, 4096))
			{
				// "EF BB BF" �������ֽھͽ�BOM��ȫ����"Byte Order Mard"��
				// ��utf8�ļ��г���BOM����������ļ���UTF-8�ļ�����BOM�ı�������utf16���á�
				if (bExceptBOM && (BYTE) strBuffer[0] == 0xEF && (BYTE) strBuffer[1] == 0xBB && (BYTE) strBuffer[2] == 0xBF)
				{
					strResult += (strBuffer + 3);
				}
				else
				{
					strResult += strBuffer;
				}
				bExceptBOM = false;
			}
			
			CloseHttpFile();
		}
	}
	catch (...)
	{
		CloseHttpFile();
#if 1
		WSACleanup();

		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD( 1, 1 );
		err = WSAStartup(wVersionRequested, &wsaData);
#endif
	}

	CloseSession();
	return strResult.length();
} 


int CHttpJsonReader::PostUrlContent(LPCSTR strHref, std::string& strResult, LPCSTR strParam,
									CString* pcsContentType)
{   
	CString httpsource = "";
	CInternetSession session1;
	CHttpConnection* conn1 = NULL;
	CHttpFile* pHttpFile = NULL;

	CString strServerName;
	CString strObject;
	WORD wPort;
	DWORD dwServiceType;
	AfxParseURL(strHref, dwServiceType, strServerName, strObject, wPort);

	DWORD retCode;
	try
	{
		conn1 = session1.GetHttpConnection(strServerName, wPort);
		pHttpFile = conn1->OpenRequest(0, strObject, NULL, 1, NULL, "HTTP/1.1",
										INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT);
		pHttpFile->AddRequestHeaders("Content-Type:application/x-www-form-urlencoded");
		pHttpFile->AddRequestHeaders("Accept:*/*");
		pHttpFile->SendRequest(NULL, 0, (void*) strParam, strlen(strParam) + 1);
		pHttpFile->QueryInfoStatusCode(retCode);
	}
	catch (CInternetException* pException)
	{
		pException->Delete();
	}

	if (pHttpFile)
	{
		if (pcsContentType)
		{
			// Content-Type: text/html; charset=gbk
			m_pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_TYPE, *pcsContentType);
		}

		bool bExceptBOM = true;
		char strBuffer[10240] = {0};
		while (pHttpFile->ReadString(strBuffer, 10240))
		{
			// "EF BB BF" �������ֽھͽ�BOM��ȫ����"Byte Order Mard"��
			// ��utf8�ļ��г���BOM����������ļ���UTF-8�ļ�����BOM�ı�������utf16���á�
			if (bExceptBOM && (BYTE) strBuffer[0] == 0xEF && (BYTE) strBuffer[1] == 0xBB && (BYTE) strBuffer[2] == 0xBF)
			{
				strResult += (strBuffer + 3);
			}
			else
			{
				strResult += strBuffer;
			}
			bExceptBOM = false;
		}
		
		pHttpFile->Close();
		delete pHttpFile;
	}
	
	conn1->Close();
	delete conn1;
	session1.Close();

	return strResult.length();
}   
    

/************************************************************************/
/* UTF8��ת��ΪANSI��, strANSIΪ��ʱ���Զ������ڴ�                      */
/************************************************************************/
bool CHttpJsonReader::UTF8Str2AnsiStr(const char* strUTF8, char** pstrANSI)
{
	bool bSuccees = false;
	if (strUTF8)
	{
		if (strUTF8[0])
		{
			//Ԥת�����õ�����ռ�Ĵ�С
			int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, strUTF8, strlen(strUTF8), NULL, 0);
			
			//����ռ�Ҫ��'\0'�����ռ䣬MultiByteToWideChar�����'\0'�ռ�
			wchar_t* wszString = new wchar_t[wcsLen + 1];
			
			//ת��
			::MultiByteToWideChar(CP_UTF8, NULL, strUTF8, strlen(strUTF8), wszString, wcsLen);
			
			//������'\0'
			wszString[wcsLen] = '\0';
			
			//Ԥת�����õ�����ռ�Ĵ�С������õĺ��������������෴
			int ansiLen = ::WideCharToMultiByte(CP_ACP, NULL, wszString, wcslen(wszString), NULL, 0, NULL, NULL);
			//ͬ�ϣ�����ռ�Ҫ��'\0'�����ռ�
			
			SAFE_DEL_ARRAY(*pstrANSI);
			*pstrANSI = new char[ansiLen + 1];
			if (*pstrANSI)
			{
				//ת��
				//unicode���Ӧ��strlen��wcslen
				::WideCharToMultiByte(CP_ACP, NULL, wszString, wcslen(wszString), *pstrANSI, ansiLen, NULL, NULL);
				
				//������'\0'
				(*pstrANSI)[ansiLen] = '\0';
				bSuccees = true;
			}
			
			delete wszString;
		}
		else
		{
			SAFE_DEL_ARRAY(*pstrANSI);
			*pstrANSI = new char[2];
			if (*pstrANSI)
			{
				(*pstrANSI)[0] = '\0';
				bSuccees = true;
			}
		}
	}
	return bSuccees;
}

/*******************************************************************************/
/* ��json�ַ���ת��ΪANSI��, strANSIΪ��ʱ���Զ������ڴ�, isUTF8�Ƿ�ΪUFT8���� */
/*******************************************************************************/
bool CHttpJsonReader::CopyJsonStr(Json::Value& val, char** pstrANSI, bool isUTF8)
{
	if (val.isString())
	{
		if (isUTF8)
		{
			return UTF8Str2AnsiStr(val.asCString(), pstrANSI);
		}
		else
		{
			return CopyAnsiJsonStr(val, pstrANSI);
		}
	}
	return false;
}

int CHttpJsonReader::ParseJsonInt(Json::Value& val)
{
	int nVal = 0;
	if (val.isInt())
	{
		nVal = val.asInt();
	}
	else if (val.isString() && val.asCString()[0] != 0)
	{
		nVal = atol(val.asCString());
	}
	return nVal;
}

UINT CHttpJsonReader::ParseJsonUInt(Json::Value& val)
{
	UINT unVal = 0;
	if (val.isInt())
	{
		unVal = val.asUInt();
	}
	else if (val.isString() && val.asCString()[0] != 0)
	{
		unVal = atol(val.asCString());
	}
	return unVal;
}

double CHttpJsonReader::ParseJsonDouble(Json::Value& val)
{
	double dVal = 0.0;
	if (val.isInt())
	{
		dVal = val.asUInt();
	}
	else if (val.isDouble())
	{
		dVal = val.asDouble();
	}
	else if (val.isString() && val.asCString()[0] != 0)
	{
		dVal = atof(val.asCString());
	}
	return dVal;
}

/************************************************************************/
/* ��ANSI��json�ַ������Ƶ�strANSI��                                    */
/************************************************************************/
bool CHttpJsonReader::CopyAnsiJsonStr(Json::Value& val, char** pstrANSI)
{
	if (val.isString())
	{
		const char* str = val.asCString();
		SAFE_DEL_ARRAY(*pstrANSI);
		*pstrANSI = new char[strlen(str) + 1];
		if (*pstrANSI)
		{
			strcpy(*pstrANSI, str);
			return true;
		}
	}
	return false;
}

/************************************************************************/
/* ����ת��Ϊʱ��time_t                                                 */
/************************************************************************/
time_t CHttpJsonReader::GetTimeFromStr(const char* strTime, BYTE byFormat)
{
	int nYear = 0;
	int nMonth = 0;
	int nDay = 0;
	int nHour = 0;
	int nMin = 0;
	int nSec = 0;

	switch (byFormat)
	{
		case TIME_FORMAT_DAY:
		{
			sscanf(strTime, "%d-%d-%d", &nYear, &nMonth, &nDay);
			break;
		}
		case TIME_FORMAT_HOUR:
			{
				sscanf(strTime, "%d-%d-%d %d", &nYear, &nMonth, &nDay, &nHour);
				break;
			}
		case TIME_FORMAT_MINUTE:
		{
			sscanf(strTime, "%d-%d-%d %d:%d", &nYear, &nMonth, &nDay, &nHour, &nMin);
			break;
		}
		case TIME_FORMAT_SECOND:
		{
			sscanf(strTime, "%d-%d-%d %d:%d:%d", &nYear, &nMonth, &nDay, &nHour, &nMin, &nSec);
			break;
		}
	}

	CTime time = CTime(nYear, nMonth, nDay, nHour, nMin, nSec);
	return time.GetTime();
}