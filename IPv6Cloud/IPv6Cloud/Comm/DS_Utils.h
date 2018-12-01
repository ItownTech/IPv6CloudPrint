#ifndef UTILS_H_
#define UTILS_H_

#include <string>
using namespace std;

class CDSUtils
{
public:
	//////////////////////////////////////////////////////////////////////////
	// ���ַ���תΪ UTF-8 ����                                              //
	// CP_ACP=ANSI,CP_UTF8=utf-8                                            //
	//////////////////////////////////////////////////////////////////////////
	static void ToUft8(CString &str,int sourceCodepage,int targetCodepage)
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
	
	//////////////////////////////////////////////////////////////////////////
	//��ȡӦ�ó���ȫ·��                                                    //
	//////////////////////////////////////////////////////////////////////////
	static void GetExePath(char *cpExePath)
	{
		//��ó���ģ�����ƣ�ȫ·��
		TCHAR exeFullPath[MAX_PATH]; 
		GetModuleFileName(NULL,exeFullPath,MAX_PATH);
		
		//��ó���ģ�������ļ���·��
		string strFullExeName(exeFullPath);
		int nLast=strFullExeName.find_last_of("\\");
		strFullExeName=strFullExeName.substr(0,nLast+1);
		
		//��������ļ�ȫ·��
		sprintf(cpExePath, "%s",strFullExeName.c_str());
	}
	
	//////////////////////////////////////////////////////////////////////////
	//��ȡϵͳʱ��                                                    //
	//////////////////////////////////////////////////////////////////////////
	static void GetDsCurrentTime(char *cpTime, int nFlag)
	{
		switch(nFlag) {
		case 0:
			{
				CTime t = CTime::GetCurrentTime();
				CString strTime = t.Format("%Y-%m-%d %H:%M:%S");
				sprintf(cpTime, "%s", strTime);
			}
			break;
		case 1:
			{
				CTime t = CTime::GetCurrentTime();
				CString strTime = t.Format("%Y��%m��%d�� %H:%M:%S");
				sprintf(cpTime, "%s", strTime);
			}
			break;
		}
	}
	
	/*******************************************************
	* �ָ��ַ��� 
	* lpszStr Դ�ַ�����lpszSplit �ָ��ַ�����bAllowNullString �ָ�ʧ�ܺ��Ƿ񷵻�Դ�ַ���
	* rArrString �ָ����ַ�������
	* �ַ������鳤�� �ɹ�
	******************************************************/
	static int SplitString(LPCTSTR lpszStr, LPCTSTR lpszSplit, CStringArray& rArrString, BOOL bAllowNullString)   
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

	//////////////////////////////////////////////////////////////////////////
	//��������                                                            //
	//////////////////////////////////////////////////////////////////////////
	static int GetRand(int nMax)
	{
		srand((unsigned)time(NULL));
		return (rand()%nMax);
	}

	//////////////////////////////////////////////////////////////////////////
	//��������                                                            //
	//////////////////////////////////////////////////////////////////////////
	static CTime CStringToCTime(CString strTime)
	{
		int nYear, nMonth, nDate, nHour, nMin, nSec;
		sscanf(strTime, "%d-%d-%d %d:%d:%d", &nYear, &nMonth, &nDate, &nHour, &nMin, &nSec);
		CTime tTime(nYear, nMonth, nDate, nHour, nMin, nSec);
		return tTime;
	}

	//��������
	static int CreateRun(char * szFileName)
	{
		//������´���
		HKEY   hKey; 
		//�ҵ�ϵͳ�������� 
		LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
		//��������Key 
		long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
		if(lRet== ERROR_SUCCESS)
		{
			//���ע��
			RegSetValueEx(hKey, _T("IPv6Cloud"), 0,REG_SZ,(const BYTE*)(LPCSTR)szFileName, MAX_PATH);
			RegCloseKey(hKey); 
		}
		return 0;
	}

	//ȡ����������
	static int DeleteRun(char * szFileName)
	{
		//������´���
		HKEY   hKey; 
		char pFileName[MAX_PATH] = {0}; 
		//�õ����������ȫ·�� 
		DWORD dwRet = GetModuleFileNameW(NULL, (LPWCH)pFileName, MAX_PATH); 
		//�ҵ�ϵͳ�������� 
		LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
		//��������Key 
		long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
		if(lRet== ERROR_SUCCESS)
		{
			//ɾ��ע��
			RegDeleteValue(hKey,_T("IPv6Cloud"));
			RegCloseKey(hKey); 
		}
		return 0;
	}
};
#endif /*UTILS_H_*/

 
