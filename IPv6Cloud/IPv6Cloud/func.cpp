// Filename: OutPrn.cpp
#include "stdafx.h"
#include <ctype.h>
#include <string.h>
#include <winspool.h>

BOOL SetSystemDefaultPrinter(LPTSTR pPrinterName);
bool StartPrint(char * szFilePath, int nCount);

// ����1
BOOL SetSystemDefaultPrinter(LPTSTR pPrinterName) 
{ 
	BOOL bFlag = FALSE; 
	LONG lResult = 0; 
	DWORD dwNeeded = 0; 
	LPTSTR pBuffer = NULL; 
	HANDLE hPrinter = NULL; 
	OSVERSIONINFO stOsvInfo = {0}; 
	PRINTER_INFO_2* pstPrintInfo2 = NULL; 

	stOsvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); 
	GetVersionEx(&stOsvInfo); 

	if(!pPrinterName) 
	{ 
		return FALSE; 
	} 

	if(stOsvInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) //Win9x 
	{ 
		// Open this printer so we can get information about it 
		bFlag = OpenPrinter(pPrinterName, &hPrinter, NULL); 
		if(!bFlag || hPrinter==NULL) 
		{ 
			return FALSE; 
		} 

		GetPrinter(hPrinter, 2, 0, 0, &dwNeeded); 
		if(dwNeeded == 0) 
		{ 
			ClosePrinter(hPrinter); 
			return FALSE; 
		} 

		pstPrintInfo2 = (PRINTER_INFO_2 *)GlobalAlloc(GPTR, dwNeeded); 
		if(pstPrintInfo2 == NULL) 
		{ 
			ClosePrinter(hPrinter); 
			return FALSE; 
		} 

		bFlag = GetPrinter(hPrinter, 2, (LPBYTE)pstPrintInfo2, dwNeeded, &dwNeeded); 
		if(!bFlag) 
		{ 
			ClosePrinter(hPrinter); 
			GlobalFree(pstPrintInfo2); 
			return FALSE; 
		} 

		// Set default printer attribute for this printer... 
		pstPrintInfo2->Attributes |= PRINTER_ATTRIBUTE_DEFAULT; 
		bFlag = SetPrinter(hPrinter, 2, (LPBYTE)pstPrintInfo2, 0); 
		if(!bFlag) 
		{ 
			ClosePrinter(hPrinter); 
			GlobalFree(pstPrintInfo2); 
			return FALSE; 
		} 

		lResult = 
			SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0L, (LPARAM)(LPCTSTR)"windows", SMTO_NORMAL, 1000, 

			NULL); 
	} 
	else if (stOsvInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) 
	{ 
#if(WINVER >= 0x0500) 
		if(stOsvInfo.dwMajorVersion >= 5) // Windows 2000 or later... 
		{ 
			bFlag = SetDefaultPrinter(pPrinterName); 
			if(!bFlag) 
			{ 
				return FALSE; 
			} 
		} 
		else // NT4.0 or earlier... 
#endif 
		{ 
			bFlag = OpenPrinter(pPrinterName, &hPrinter, NULL); 
			if(!bFlag || hPrinter==NULL) 
			{ 
				return FALSE; 
			} 

			GetPrinter(hPrinter, 2, 0, 0, &dwNeeded); 
			if(dwNeeded == 0) 
			{ 
				ClosePrinter(hPrinter); 
				return FALSE; 
			} 

			pstPrintInfo2 = (PRINTER_INFO_2*)GlobalAlloc(GPTR, dwNeeded); 
			if(pstPrintInfo2 == NULL) 
			{ 
				ClosePrinter(hPrinter); 
				return FALSE; 
			} 

			bFlag = GetPrinter(hPrinter, 2, (LPBYTE)pstPrintInfo2, dwNeeded, &dwNeeded); 
			if((!bFlag) || (!pstPrintInfo2->pDriverName) || (!pstPrintInfo2->pPortName)) 
			{ 
				ClosePrinter(hPrinter); 
				GlobalFree(pstPrintInfo2); 
				return FALSE; 
			} 

			pBuffer = 
				(LPTSTR)GlobalAlloc(GPTR, lstrlen(pPrinterName)+lstrlen(pstPrintInfo2->pDriverName)+lstrlen(pstPrintInfo2->pPortName)+3); 
			if(pBuffer == NULL) 
			{ 
				ClosePrinter(hPrinter); 
				GlobalFree(pstPrintInfo2); 
				return FALSE; 
			} 

			// Build string in form "printername,drivername,portname" 
			lstrcpy(pBuffer, pPrinterName); lstrcat(pBuffer, ","); 
			lstrcat(pBuffer, pstPrintInfo2->pDriverName); lstrcat(pBuffer, ","); 
			lstrcat(pBuffer, pstPrintInfo2->pPortName); 

			bFlag = WriteProfileString("windows", "device", pBuffer); 
			if(!bFlag) 
			{ 
				ClosePrinter(hPrinter); 
				GlobalFree(pstPrintInfo2); 
				GlobalFree(pBuffer); 
				return FALSE; 
			} 
		} 

		lResult = SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0L, 0L, SMTO_NORMAL, 1000, NULL); 
	} 

	if(hPrinter) 
	{ 
		ClosePrinter(hPrinter); 
	} 
	if(pstPrintInfo2) 
	{ 
		GlobalFree(pstPrintInfo2); 
	} 
	if(pBuffer) 
	{ 
		GlobalFree(pBuffer); 
	} 

	return TRUE; 
}

bool StartPrint(char * szFilePath, int nCount)
{
	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = TEXT("print");
	ShExecInfo.lpFile = TEXT(szFilePath); //�˴��Ǵ���ӡ���ĵ� 
	ShExecInfo.lpParameters = TEXT(""); 
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;

	bool bRet = false;
	for(int i=0; i<nCount; i++)
	{
		bRet = ShellExecuteEx(&ShExecInfo); //����� bool�������ж��Ƿ�ɹ���
		if(!bRet)
		{
			return false;
		}
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	}
	return bRet;
}
// ����2
//// *******�������********
//// ����ʵ����ָ��ADDJOB_INFO_1
//// �ṹ��ָ��
//LPBYTE  pJob=0; 
////��ӡ�����
//HANDLE m_hPrinter=NULL; 
////********��������****************
////��ô�ӡ��ҵ����ʱ�ļ�����ID��
////�������ڱ���pJob���ڵĿռ䣬�ɹ���
////��true,ʧ�ܷ���false
//bool GetSpoolFileName( ); 
////֪ͨϵͳ����׼�����������������
////ͬʱ�ͷź���������ռ�õ��ڴ�
//void EndPrint( ) ;
////��������ʾ������������ĺ�����
////��ɽ�һ���ļ������Ĳ���
////�Ĵ����ⲿ�豸
//void Demo( );
//
//bool GetDefaultPrinterName(CString &name)
//{
//	CPrintDialog pd(TRUE);
//	if(pd.GetDefaults()==FALSE)
//	{
//		AfxMessageBox("Windowsϵͳû�а�װȱʡ��ӡ��");
//		return false;
//	}
//	name=pd.GetDeviceName();
//	if (pd.m_pd.hDevNames)
//	{
//		::GlobalUnlock(pd.m_pd.hDevNames);
//		::GlobalFree(pd.m_pd.hDevNames);
//		pd.m_pd.hDevNames=NULL;
//	}
//	if (pd.m_pd.hDevMode)
//	{
//		::GlobalFree(pd.m_pd.hDevMode);
//		pd.m_pd.hDevMode=NULL;
//	}
//	return true;
//}
//
////********* ��������ʵ�� ***********
//bool GetSpoolFileName(CString name)
//{
//	//����һЩ��ʱ����
//	DWORD dwNeeded=0;
//	DWORD dwReturned=0;
//	LPBYTE pPrinterEnum=0;
//	BOOL nRet=FALSE;
//	//CString name;
//	//���ϵͳȱʡ��ӡ���������ȵ���EnumPrinters�����Ҫ
//	//���Ĵ洢�ռ����Ż�õ���Ϣ��
//	//�ô�Сд�����dwNeeded ��
//	//if(GetDefaultPrinterName(name)==false) return false;
//	::EnumPrinters(PRINTER_ENUM_NAME,NULL,2,NULL,0,&dwNeeded,&dwReturned);
//	if(dwNeeded<=0) return false;
//    //����ǰ����������洢�ռ�
//	pPrinterEnum=new BYTE[dwNeeded];
//	//��һ�ε��ú���EnumPrinters��
//	//�����ϵͳȱʡ��ӡ��
//	//��Ϣ����pPrinterEnum�С�
//	nRet=::EnumPrinters(PRINTER_ENUM_NAME,NULL,2,pPrinterEnum
//		,dwNeeded,&dwNeeded,&dwReturned);
//	if(nRet==FALSE ||dwReturned==0)
//	{
//		//û���ҵ�����Ҫ��ȱʡ��ӡ������������
//		delete pPrinterEnum;
//		return false;
//	}
//	// ��pPrinterEnumת��Ϊ�ṹ PRINTER_INFO_2��ָ��
//	PRINTER_INFO_2 *pInfo=(PRINTER_INFO_2 *)pPrinterEnum;
//	DWORD num;
//	for(num=0L;num<dwReturned;num++)
//	{
//		if(lstrcmp((LPTSTR)(&(pInfo[num].pDevMode->dmDeviceName[0])),name)==0){ break;}
//	}
//	if(num>=dwReturned) 
//	{
//		AfxMessageBox("û���ҵ���Ӧ�Ĵ�ӡ��");
//		return false;
//	}
//	//���ݽṹPRINTER_INFO_2�а����Ĵ�ӡ���������򿪸ô�ӡ����
//	//����õľ�������ڱ��� m_hPrinter��
//	if(!::OpenPrinter(pInfo[num].pPrinterName,&m_hPrinter,NULL))
//	{   //�򿪴�ӡ��ʧ�ܣ���������
//		AfxMessageBox("�򿪴�ӡ��ʧ��");
//		//�ͷ��ڴ�
//		delete pPrinterEnum;
//		m_hPrinter=NULL;
//		return false;
//	}
//	//���治����Ҫ���ͷ���ռ�õ��ڴ�
//	delete pPrinterEnum;
//	//ʹ�ú���AddJob ���������ӵĴ�ӡ��ҵ����ʱ�ļ����Ͷ�Ӧ��ID��
//	dwNeeded=0;
//	ASSERT(pJob==NULL);
//	//����ռ�������Žṹ ADDJOB_INFO_1����������Ϣע�ⲻҪ����AddJob�����Զ���
//	//����Ҫ���Ŀռ�����
//	pJob=new BYTE[2048];
//	//��ADDJOB_INFO_1����������Ϣ���ú����ķ���ֵ���˲����˼���
//	//������ȷ�Լ���������2K���ڴ��㹻�ˡ�
//	BOOL flag=::AddJob( m_hPrinter,1,pJob,2048,&dwNeeded);
//	if(!flag)
//	{//�������ɹ�����
//		delete []pJob;//�ͷ��ڴ�
//		pJob=0;
//		AfxMessageBox("�����ڴ�ʧ��");
//		::ClosePrinter( m_hPrinter);
//		m_hPrinter=NULL;
//		return false;
//	}
//	return true;
//}
//
//// ***************************
//void EndPrint( )
//{
//	ASSERT(pJob);
//	//������Ϣ����ӡ�������������
//	//ǰ����ҵ���������	
//	::ScheduleJob( m_hPrinter,((ADDJOB_INFO_1 *)pJob)->JobId );
//	//�ͷŴ�ӡ���
//	ClosePrinter( m_hPrinter);
//	m_hPrinter=0;
//	delete []pJob;//�ͷ��ڴ�
//	pJob=0;
//	m_hPrinter=0;
//}
//
//// ****************************
//void Demo(CString strPrintName, CString strFileName)
//{
//	//���ú���������ʱ�ļ���JOB
//	if(!GetSpoolFileName(strPrintName)) return;
//
//	//��ԭʼ���ݷ�����ʱ�ļ���
//	CopyFile(strFileName, ((ADDJOB_INFO_1 *)pJob)->Path, FALSE);
//
//	//֪ͨ��������ҵ׼������
//	EndPrint( );
//}