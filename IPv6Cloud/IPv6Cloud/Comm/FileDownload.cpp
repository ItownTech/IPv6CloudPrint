////////////////////////////////////////////////////////////////////////////////
//  File Name: XDMediaFileDownload.cpp
////////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FileDownload.h"
#include <sys/stat.h>
#include <io.h>
#include "../IPv6CloudDlg.h"

extern char* UrlEncode(const char* strUrlGbk);

CFileDownload::CFileDownload()
{
	m_pHttpDownload = NULL;
	m_dwThreadIdDownload = 0;
	m_hDownload = NULL;
	m_hMutexDownloadList = CreateMutex(NULL, FALSE, NULL);
	m_hEventAddDownload = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventDownloadFinished = NULL;
	m_bDownloadExit = false;
}

CFileDownload::~CFileDownload()
{
	ClearDownloadMediaFileItems();
	StopDownloadThread();
	SAFE_DEL(m_pHttpDownload);
	SAFE_CLOSE_HANDLE(m_hMutexDownloadList);
	SAFE_CLOSE_HANDLE(m_hEventAddDownload);

}

void CFileDownload::StopDownloadThread()
{
	if (m_hDownload)
	{
		m_bDownloadExit = true;
		m_pHttpDownload->Cancel();
		int t = 2000;
		DWORD ExitCode;
		BOOL bEnd = FALSE;
		PostThreadMessage(m_dwThreadIdDownload, WM_QUIT, 0, 0);
		while (t--)
		{
			GetExitCodeThread(m_hDownload, &ExitCode);
			if (ExitCode != STILL_ACTIVE)
			{
				bEnd = TRUE;
				break;
			}
			else
			{
				Sleep(10);
			}
		}
		if (!bEnd)
		{
			TerminateThread(m_hDownload, 0);
		}
		m_hDownload = NULL;
	}
	else
	{
	}

	SAFE_DEL(m_pHttpDownload);
	SAFE_CLOSE_HANDLE(m_hEventDownloadFinished);
}


bool CFileDownload::StartDownloadThread()
{
	StopDownloadThread();

	m_hEventDownloadFinished = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_pHttpDownload = new CHttpDownload();
	m_pHttpDownload->SetDownloadFinishedEvent(m_hEventDownloadFinished);

	m_hDownload = CreateThread(0, 0, dwDownloadThreadProc, this, 0, &m_dwThreadIdDownload);
	if (!m_hDownload)
	{
		//MessageBox("���������߳�ʧ��!", "��ʾ", MB_OK);
		SAFE_DEL(m_pHttpDownload);
		SAFE_CLOSE_HANDLE(m_hEventDownloadFinished);
		return false;
	}
	return true;
}

DWORD WINAPI CFileDownload::dwDownloadThreadProc(LPVOID lpParameter)
{
	CFileDownload* pMediaDownload = (CFileDownload*) lpParameter;
	CHttpDownload* pHttpDownload = pMediaDownload->m_pHttpDownload;

	//char strRemotURL[400]; 
	//char strLocalFileName[400];
	//GetModuleFileName(NULL, strLocalFileName, 400);
	//char* pstrRelativePath = strrchr(strLocalFileName, '\\');
	//pstrRelativePath++;
	//
	//bool bFileExistAndValid = false;
	FileItem* pMFI = NULL;
	while (!pMediaDownload->m_bDownloadExit)
	{
		pMFI = pMediaDownload->GetDownloadMediaFileItem();
		if (pMFI)
		{
			//pMFI->GetLocalFile(pstrRelativePath);
			if (!pMediaDownload->m_bDownloadExit && !pMFI->bLocal && strlen(pMFI->strSvrUrl)>0)
			{
				//// �ļ�������,����
				// ��Ⲣ����Ŀ¼
				CheckCreateDirectory(pMFI->szLocalFilePath);
				
				OutputDebugString("\n");
				OutputDebugString(pMFI->strSvrUrl);
				OutputDebugString("\n");

				//CString csUrlEncode;
				//char pstrUrl[MAX_PATH] = {0};
				//strcpy(pstrUrl, pMFI->strSvrUrl);
				//// pMFI->strUrl����������ַ,��Э��ͷ(http://)��Ҫ����URL����
				//char* strProtocol = strstr(pstrUrl, "://");
				//char* strAddr = strProtocol + 3;
				//int nLen = strAddr - pstrUrl;
				//char szProtocol[256];

				//strncpy(szProtocol, pstrUrl, nLen);
				//szProtocol[nLen] = 0;

				//csUrlEncode += szProtocol;
				//char *pUrl = UrlEncode(strAddr);
				//csUrlEncode += pUrl;
				
				pHttpDownload->Init(NULL, pMFI->strSvrUrl, pMFI->szLocalFilePath);
				//delete pUrl;
				if (pHttpDownload->Start())
				{
					// �ȴ��������
					pHttpDownload->WaitDownloadFinished(INFINITE);
					if (pHttpDownload->m_bDownloadSucceed && !pHttpDownload->IsCancel())
					{
						pMFI->bLocal = true;
					}
					else
					{
						pMFI->bLocal = false;
					}

					// ��ɺ�Ĵ���
					if(pMFI->pParent != NULL)
					{
						((CIPv6CloudDlg *)pMFI->pParent)->UpdateUnOrderListStatus(pMFI, pMFI->bLocal, false);
					}
				}
				else
				{
					pMFI->bLocal = false;

					// ��ɺ�Ĵ���
					if(pMFI->pParent != NULL)
					{
						((CIPv6CloudDlg *)pMFI->pParent)->UpdateUnOrderListStatus(pMFI, pMFI->bLocal, false);
					}
				}
				pHttpDownload->DisInit();
			}
			
			//SDLOG_PRINTF("dwDownloadThreadProc", SD_LOG_LEVEL_DEBUG, "dwDownloadThreadProc %s download end", pMFI->m_strUrl);

			if (pMFI->bDeleteOnDownFinish)
			{
				// ������ɺ��Զ�ɾ��
				delete pMFI;
				pMFI = NULL;
			}
		}
	}

	return 0;
}


FileItem* CFileDownload::GetDownloadMediaFileItem()
{
	FileItem* pMFI = NULL;	
	if (WaitForSingleObject(m_hMutexDownloadList, 2000) == WAIT_OBJECT_0)
	{
		if (m_downloadMFIs.empty())
		{
			ResetEvent(m_hEventAddDownload);
			ReleaseMutex(m_hMutexDownloadList);
			if (WaitForSingleObject(m_hEventAddDownload, 2000) == WAIT_OBJECT_0 && 
				WaitForSingleObject(m_hMutexDownloadList, 2000) == WAIT_OBJECT_0)
			{
				if (!m_downloadMFIs.empty())
				{
					pMFI = *(m_downloadMFIs.begin());
					m_downloadMFIs.pop_front();
				}
				ReleaseMutex(m_hMutexDownloadList);
			}
		}
		else
		{
			pMFI = *(m_downloadMFIs.begin());
			m_downloadMFIs.pop_front();
			ReleaseMutex(m_hMutexDownloadList);
		}
	}
	return pMFI;
}


/************************************************************************/
/* ����Ҫ���ص�FileItem��                                          */
/************************************************************************/
void CFileDownload::AddDownloadMediaFileItem(FileItem* pMFI, bool bAddFront)
{
	if (pMFI && WaitForSingleObject(m_hMutexDownloadList, INFINITE) == WAIT_OBJECT_0)
	{
		if (bAddFront)
		{
			m_downloadMFIs.push_front(pMFI);
		}
		else
		{
			m_downloadMFIs.push_back(pMFI);
		}


		ReleaseMutex(m_hMutexDownloadList);
		SetEvent(m_hEventAddDownload);

		if (!m_hDownload)
		{
			StartDownloadThread();
		}
	}
}

/************************************************************************/
/* ���������Ŀ                                                         */
/************************************************************************/
void CFileDownload::ClearDownloadMediaFileItems()
{
	if (WaitForSingleObject(m_hMutexDownloadList, INFINITE) == WAIT_OBJECT_0)
	{
		
		FileItem* pMFI = NULL;	
		list<FileItem*>::iterator iter;
		for (iter = m_downloadMFIs.begin(); iter != m_downloadMFIs.end(); iter++)
		{
			pMFI = *iter;
			if (pMFI->bDeleteOnDownFinish)
			{
				delete pMFI;
			}
		}
		m_downloadMFIs.clear();
		
		ReleaseMutex(m_hMutexDownloadList);
		ResetEvent(m_hEventAddDownload);
	}
}



/************************************************************************/
/* ����ļ��Ƿ�������ֽ�����Ϊ0                                        */
/************************************************************************/
bool CFileDownload::FileExistAndNotEmpty(char* szPath)
{
	int hasFile = access(szPath, 0);
	if (hasFile == -1)
	{
		return false;
	}

	struct _stat info;
	_stat(szPath, &info);
	return info.st_size > 0;
}

/************************************************************************/
/* ��Ⲣ����Ŀ¼szPath,ֻ���szRelativePath�Ĳ���                      */
/************************************************************************/
void CFileDownload::CheckCreateDirectory(char* szPath)
{
	//��ó���ģ�������ļ���·��
	string strFullExeName(szPath);
	int nLast=strFullExeName.find_last_of("\\");
	strFullExeName=strFullExeName.substr(0,nLast+1);

	if (access(strFullExeName.c_str(), 0) == -1)
	{
		CreateDirectory(strFullExeName.c_str(), NULL);
	}
}