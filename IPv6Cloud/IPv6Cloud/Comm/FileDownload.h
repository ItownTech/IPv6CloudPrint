////////////////////////////////////////////////////////////////////////////////
//  File Name: XDMediaFileDownload.cpp
////////////////////////////////////////////////////////////////////////////////

#ifndef _XD_MediaFile_DownLoad_H_
#define _XD_MediaFile_DownLoad_H_

#include <list>
#include "HttpDownload.h"
#include <string.h>

#define  DEFAULT_DOWNLOAD_FILE_PATH       "DownloadFiles"
#define  DEFAULT_CACHE_FILE_PATH          "CacheFiles"

struct FileItem
{
	char    orderId[STRING_LEN_TINY];               // ����id
	int     nDocId;                 // �ļ�id
	int 	byType;					// ý���ļ����ͣ���ʱֻ������ͼ��������������һЩ�������ļ���
	bool	bLocal;					// �Ƿ����ص��˱���
	bool	bDeleteOnDownFinish;	// ������ɺ����free��FileItem����
	char	strSvrUrl[MAX_PATH];	// URL:web�������ϵ�url�������ص����غ���ļ�·��(���ļ���)
	char    szLocalFilePath[MAX_PATH];
	void    *pParent;

	bool    bPrint;
};

class CFileDownload
{
public:
	CFileDownload();
	virtual ~CFileDownload();

	// ����Ҫ���ص�FileItem��
	void AddDownloadMediaFileItem(FileItem* pMFI, bool bAddFront = false);

	// ȥ��ָ�����͵�������Ŀ
	void RemoveDownloadMediaFileItemsByType(BYTE* byType, int nTypeCount);


public:

	//// ���ع���
	bool StartDownloadThread();
	void StopDownloadThread();
	FileItem* GetDownloadMediaFileItem();
	void ClearDownloadMediaFileItems();
	static DWORD WINAPI dwDownloadThreadProc(LPVOID lpParameter);

	static bool FileExistAndNotEmpty(char* szPath);
	static void CheckCreateDirectory(char* szPath);
	
	DWORD                  m_dwThreadIdDownload;
	HANDLE                 m_hDownload;
	list<FileItem*>        m_downloadMFIs; // Ҫ���ص�ý���ļ��б�,����m_mediaFiles�������Ա��pMediaFiles,������غ�ȥ��
	HANDLE                 m_hMutexDownloadList;
	HANDLE                 m_hEventAddDownload;
	HANDLE                 m_hEventDownloadFinished;
	CHttpDownload*         m_pHttpDownload;
	bool                   m_bDownloadExit;

};



#endif //#ifndef _XD_MediaFile_DownLoad_H_
