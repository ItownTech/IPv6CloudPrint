
// IPv6CloudSub.cpp : Main��ʵ���࣬�޹ؽ�Ҫ�Ķ�����ϵͳ��ض�����д���������Ӧ�ó����߼��Ķ�
//

#include "stdafx.h"
#include "IPv6Cloud.h"
#include "IPv6CloudDlg.h"
#include "afxdialogex.h"
#include "PubHead.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/********************************** �̺߳��� **************************************/
void CIPv6CloudDlg::FreeMemSvc(void * pParam)
{
	CIPv6CloudDlg *pThis = (CIPv6CloudDlg *)pParam;
	
	while (pThis->m_bWork)
	{
		pThis->ToMemFree();
		if (pThis->m_nFreeMemTime < 60)
		{
			pThis->m_nFreeMemTime = 60;
		}
		
		Sleep(pThis->m_nFreeMemTime * 1000);
	}
}

void CIPv6CloudDlg::DataArrivedCB(char *data, int length, DWORD userdata)
{
	TRACE("recv len:%d, date:%s\n", length, data);

	CIPv6CloudDlg *pThis = (CIPv6CloudDlg *)userdata;
	if(strstr(data, "offline:1") != NULL)
	{
		pThis->ShowTrayTips("ϵͳ��ʾ", "���˺��������ط���¼��");

		CTime tNow = CTime::GetCurrentTime();
		char szMsg[256] = {0};
		sprintf(szMsg, "����˺���%d��%d������һ̨�����ϵ�¼����Ǳ��˲����������������й¶������ǰ�� http://www.v6cp.com �޸�����!\n�Ƿ����µ�¼��", tNow.GetHour(), tNow.GetMinute());

		if (AfxMessageBox(szMsg, MB_YESNO) != IDYES)
		{
			pThis->SetAutoLoginStatus(1);
			pThis->Restart();
		}
		else
		{
			pThis->m_LoginDlg.ForcedLogin();
			pThis->m_LoginDlg.OnOKByDui(CGlobal::g_UserName, CGlobal::g_UserPwd, CGlobal::g_Remember, CGlobal::g_AutoLogin, CGlobal::g_ServerAddr, true);
		}
	}
	else
	{
		pThis->ShowTrayTips("ϵͳ��ʾ", "�յ��µĶ������ȴ��б��ȡ��");
		pThis->GetAndRefresh(0);
	}
}

void CIPv6CloudDlg::StatusChangeCB(char *data, int length, DWORD userdata)
{
}

extern BOOL SetSystemDefaultPrinter(LPTSTR pPrinterName);
extern bool StartPrint(char * szFilePath, int nCount);
void CIPv6CloudDlg::PrintHisSvc(void * pParam)
{
	CIPv6CloudDlg *pThis = (CIPv6CloudDlg *)pParam;
	try
	{
		CString strPrintName = "";
		int nCount = pThis->m_DuiMainDlg.m_listBlackPrint->GetCount();
		for(int i=0; i<nCount; i++)
		{
			CControlUI* pChildCtrl = pThis->m_DuiMainDlg.m_listBlackPrint->GetItemAt(i);
			if(pChildCtrl != NULL)
			{
				CControlUI *labName =pThis->m_DuiMainDlg.m_pm.FindSubControlByName(pChildCtrl, _T("labName"));
				if(labName)
				{
					CString strName = labName->GetText();
					if(strName.Find("[������]") < 0)
					{
						strPrintName = strName;
						break;
					}
				}
			}
		}

		if(strPrintName == "")
		{
			pThis->ShowTrayTips("������ʾ", "��ӡ��ʷ����ʧ�ܣ���������һ̨��ӡ����");
			return;
		}

		CString strCache = "";
		strCache.Format("%s%s\\%d.xlsx", CGlobal::g_ExePath, DEFAULT_CACHE_FILE_PATH, GetTickCount());
		CString strFileName = pThis->ExportHis(strCache, 0);
	
		if(strFileName == "")
		{
			pThis->ShowTrayTips("������ʾ", "׼����ӡ����ʱʧ�ܣ�");
		}

		if(SetSystemDefaultPrinter(strPrintName.GetBuffer(0)))
		{
			char szMsg[1024] = {0};
			if(StartPrint(strFileName.GetBuffer(0),1))
			{
				pThis->ShowTrayTips("��ӡ�ɹ���", "");
			}
			else
			{
				pThis->ShowTrayTips("��ӡ�ļ�ʧ�ܣ�", "");
			}
		}
		else
		{
			pThis->ShowTrayTips("������ʾ", "��ӡ��ʷ����ʱ������Ĭ�ϴ�ӡ��ʧ�ܣ�");
		}
	}
	catch(...)
	{
		pThis->ShowTrayTips("������ʾ", "��ӡ��ʷ�����쳣��");
	}
}

void CIPv6CloudDlg::GetHisOrderListSvc(void * pParam)
{
}

void CIPv6CloudDlg::AutoPrintSvc(void * pParam)
{
	CIPv6CloudDlg *pThis = (CIPv6CloudDlg *)pParam;

	pThis->SetLoginStatus(1);
	
	//�������
	pThis->m_UnOrderListLock.Lock();
	pThis->m_UnOrderList.clear();
	pThis->m_UnOrderListLock.UnLock();
	pThis->GetUnOrderList(0);
	if(pThis->m_nTotalUnOrder > 0)
	{
		int nPageCount = pThis->m_nTotalUnOrder / MAIN_ORDER_LIST_PAGE_NUM;
		if(pThis->m_nTotalUnOrder % MAIN_ORDER_LIST_PAGE_NUM != 0)
		{
			nPageCount ++;
		}
		for(int i=1; i<nPageCount; i++)
		{
			pThis->GetUnOrderList(i);
		}
	}
	pThis->RefreshUnOrderList();

	pThis->m_HisOrderListLock.Lock();
	pThis->m_HisOrderList.clear();
	pThis->m_HisOrderListLock.UnLock();
	pThis->GetHisOrderList(0);
	if(pThis->m_nHisTotalOrder > 0)
	{
		int nPageCount = pThis->m_nHisTotalOrder / HIS_ORDER_LIST_PAGE_NUM;
		if(pThis->m_nHisTotalOrder % HIS_ORDER_LIST_PAGE_NUM != 0)
		{
			nPageCount ++;
		}
		for(int i=1; i<nPageCount; i++)
		{
			pThis->GetHisOrderList(i);
		}
	}
	pThis->RefreshHisOrderList();

	pThis->GetAccount();

	//�Զ���ӡ
	bool bFirst = true;
	int nIndex = 1;

	//�����󣬻��δ��ɶ���
	CString strUnList = "";
	while (pThis->m_bWork)
	{
		pThis->SetLoginStatus(1);

		pThis->m_PrintOrderListLock.Lock();
		pThis->m_PrintOrderList.clear();

		pThis->m_UnOrderListLock.Lock();
		int nSize = pThis->m_UnOrderList.size();
		for(int i=0; i<nSize; i++)
		{
			if(pThis->m_UnOrderList[i].printStatus == 0 && pThis->m_UnOrderList[i].specialOrder == 0)
			{
				pThis->m_PrintOrderList.push_back(pThis->m_UnOrderList[i]);
			}
		}
		pThis->m_UnOrderListLock.UnLock();

		pThis->m_HisOrderListLock.Lock();
		nSize = pThis->m_HisOrderList.size();
		for(int i=0; i<nSize; i++)
		{
			if(pThis->m_HisOrderList[i].printStatus == 0 && pThis->m_HisOrderList[i].specialOrder == 0)
			{
				pThis->m_PrintOrderList.push_back(pThis->m_HisOrderList[i]);
			}
		}
		pThis->m_HisOrderListLock.UnLock();
		pThis->m_PrintOrderListLock.UnLock();

		pThis->m_PrintOrderListLock.Lock();
		nSize = pThis->m_PrintOrderList.size();
		if(nSize > 0)
		{
			if(bFirst)
			{
				pThis->SendMessage(WM_AUTO_PRINT_MSG, NULL, NULL);
				bFirst = false;
			}

			for(int i=0; i<nSize; i++)
			{
				CString strTmp = "";
				strTmp.Format("[%s]", pThis->m_PrintOrderList[i].orderId);
				if(!pThis->m_bAutoPrint)
				{
					if(strUnList.Find(strTmp)<0)
					{
						strUnList += strTmp;
					}
				}

				if(pThis->m_bAutoPrint)
				{
					if(strUnList.Find(strTmp)<0)
					{
						pThis->RPrint(pThis->m_PrintOrderList[i]);
						Sleep(10 * 1000);
					}
				}
			}
		}
		else
		{
			bFirst = false;
		}
		pThis->m_PrintOrderListLock.UnLock();

		//���������������
		//if(nIndex % 6 == 0)
		//{
		//	pThis->GetAndRefresh(1);
		//	nIndex = 0;
		//}
		pThis->GetAccount();
		nIndex ++;
		Sleep(10*1000);
	}
}
