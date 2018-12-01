
// IPv6CloudDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "IPv6Cloud.h"
#include "IPv6CloudDlg.h"
#include "afxdialogex.h"
#include "PubHead.h"
#include <process.h>
#include "Comm/MemFree.h"
#include "Comm\MyExcel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern BOOL SetSystemDefaultPrinter(LPTSTR pPrinterName);
extern bool StartPrint(char * szFilePath, int nCount);

// CIPv6CloudDlg ��Ϣ�������
BOOL CIPv6CloudDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// ���ȫ��������Ϣ
	m_cGlobal.Init();

	//// �򿪵�¼����
	if(m_LoginDlg.DoModal() == IDOK)
	{
		//AfxMessageBox("��¼�ɹ���");
	}
	else
	{
		//AfxMessageBox("��¼ʧ�ܣ�");
		CDialogEx::OnCancel();
		return FALSE;
	}

	//
	SetAutoLoginStatus(0);

	// ��ʼ�����½�����
	InitTray();

	// ����ڴ���������Ȩ��
	EnablePrivilege();

	// ��ʼ���ڴ�����ʱ��Ϊ600��
	m_nFreeMemTime = 600;

	// �����ڴ������߳�
	_beginthread(FreeMemSvc, NULL, this);

	// ��ʼ������Ӧ�ؼ���С
	INIT_EASYSIZE;

	// ���ô��ڴ�С
	MoveWindow(0, 0, 890, 590, 1);
	
	// ���ص�¼�Ի���
	m_DuiMainDlg.Create(*this, NULL, UI_WNDSTYLE_CHILD, 0, 0, 0, 890, 590);  
	m_DuiMainDlg.ShowWindow(TRUE);
	m_DuiMainDlg.m_pParent = (void *)this;
	
	ModifyStyle(0, WS_MINIMIZEBOX);

	HRGN hRgn;  
    RECT rect;
	GetWindowRect(&rect);  
    hRgn = CreateRoundRectRgn(0, 0, rect.right - rect.left, rect.bottom - rect.top, 5, 5);  
    SetWindowRgn(hRgn, TRUE);
	CenterWindow();

	CWndShadow::Initialize(AfxGetInstanceHandle());
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	//styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    m_WndShadow.Create(m_hWnd);
	m_WndShadow.SetSize(3);
	m_WndShadow.SetPosition(0, 0);
	m_WndShadow.Show(m_hWnd);
	
	//��������
	if(!m_TCP.CreateServer(CGlobal::g_ClientPort, 5))
	{
		ShowTrayTips("������ʾ", "��CreateServer����������������");
		return FALSE;
	}

	if(!m_TCP.StartServer(StatusChangeCB, DataArrivedCB, (DWORD)this))
	{
		ShowTrayTips("������ʾ", "��StartServer�������������");
		m_TCP.Close();
		return FALSE;
	}

	CString strMsg = "v6��ӡ�ͻ��������ɹ�";
	//���ÿͻ��˵�ַ
	int nType = GetIPv6();
	if(nType < 0)
	{
		strMsg += "\n��ȡ�ͻ���IP��ַʧ�ܣ����ֶ����ÿͻ��˵�ַ";
		strcpy(m_szClientAddr, "");
	}
	else if(nType == 1)
	{
		strMsg += "\n* ��ȡIPv6��ַʧ�ܣ��ͻ�������IPv4��ַ";
	}
	SetClientAddr(m_szClientAddr);
	m_DuiMainDlg.m_editLocalIpv6->SetText(m_szClientAddr);

	//���÷���˵�ַ
	m_DuiMainDlg.m_editServerIpv6->SetText(CGlobal::g_ServerAddr);
	if(CGlobal::g_SavePath == NULL || strlen(CGlobal::g_SavePath) <= 0)
	{
		char szSavePath[MAX_PATH] = {0};
		sprintf(szSavePath, "%s%s",  CGlobal::g_ExePath, DEFAULT_DOWNLOAD_FILE_PATH);
		m_DuiMainDlg.SetSavePath(szSavePath);
	}
	else
	{
		m_DuiMainDlg.SetSavePath(CGlobal::g_SavePath);
	}
	
	//�����Զ���ӡ
	m_bAutoPrint = true;
	m_DuiMainDlg.m_btnPrintOff->SetVisible(false);
	m_DuiMainDlg.m_btnPrintOn->SetVisible(true);
	strMsg+= "\n* �Զ���ӡģʽ�ѿ���";
	ShowTrayTips("ϵͳ��ʾ", strMsg);

	m_strMainSearch = "";
	m_strHisSearch = "";       
	
	CDSPrint::GetWinPrint(m_arrPrintList);
	m_DuiMainDlg.UpdatePrint(m_arrPrintList);

	CString strUserInfo = "";
	CStringW cswNickName(m_LoginDlg.m_szNickName);
	int nLen = cswNickName.GetLength();
	if (nLen > 6)
	{
		cswNickName = cswNickName.Mid(0, 6);
		cswNickName += _T("��");
	}
	
	CString strNickName(cswNickName);
	strUserInfo.Format("�𾴵�%s{a}�˳���ǰ�˺�{/a}�����ã���ӭ����%s! ", strNickName.GetBuffer(0), DS_TRAY_TIP_STR);

	CString strWeb = "";
	strWeb.Format("�̼ҵ�ַ��{a}%s{/a}", DS_TRAY_WEB_STR);
	m_DuiMainDlg.m_btnUserInfo->SetText(strUserInfo);
	m_DuiMainDlg.m_btnMainInfo->SetText(strWeb);

	// ������ö����߳�
	//_beginthread(PrintHisSvc, NULL, this);
	//_beginthread(GetHisOrderListSvc, NULL, this);
	// �����Զ���ӡ�߳�
	_beginthread(AutoPrintSvc, NULL, this);

	if(m_LoginDlg.m_duiLoginDlg.m_bIsAutoLogin)
	{
		ShowWindow(SW_MINIMIZE);
	}

	SetForegroundWindow();
	//Restart();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

int CIPv6CloudDlg::InitTray()
{
	int nRet = RET_OK;
	try
	{
		/* ��ʼ������ */
		m_NotifyIconData.cbSize = sizeof (NOTIFYICONDATA);		//���ֽ�Ϊ��λ������ṹ�Ĵ�С 
		m_NotifyIconData.hWnd = m_hWnd;							//��������ͼ��֪ͨ��Ϣ�Ĵ��ھ��
		m_NotifyIconData.uID = IDR_MAINFRAME;						//Ӧ�ó�����ĸ�ͼ���ID��
		m_NotifyIconData.uFlags = NIF_ICON|NIF_MESSAGE|NIF_INFO|NIF_TIP;	//���ø�ͼ������ԣ�NIF_ICON�����ó�ԱhIcon��Ч ��NIF_MESSAGE�����ó�ԱuCallbackMessage��Ч ��NIF_TIP�����ó�ԱszTip��Ч 
		m_NotifyIconData.uCallbackMessage= WM_NOTIFYICON;			//Ӧ�ó��������ϢID�ţ�����Ϣ���ݸ�hWnd
		m_NotifyIconData.hIcon = m_hIcon;	
		m_NotifyIconData.dwInfoFlags=NIIF_INFO;//ͼ��ľ��
		strcpy(m_NotifyIconData.szTip, DS_TRAY_TIP_STR);		//���ͣ����ͼ������ʾ����ʾ��Ϣ  

		_tcscpy(m_NotifyIconData.szInfoTitle,"ϵͳ��ʾ");//��ʾ�ı�����Ϣ
		_tcscpy(m_NotifyIconData.szInfo,"v6��ӡ�ͻ��������ɹ���");//��ʾ���ڲ���Ϣ
		Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);

		/* ��ʼ�����̲˵� */
		m_TrayMenu.LoadMenu(IDR_TRAY_MENU); //��ȡ��Դ

		m_TrayMainBmp.LoadBitmap(IDB_IPv6Cloud_16_16_BMP);
		m_TraySyssetBmp.LoadBitmap(IDB_SYSSET_16_16_BMP);
		m_TrayExitBmp.LoadBitmap(IDB_EXIT_16_16_BMP);
		m_TrayAboutBmp.LoadBitmap(IDB_ABOUT_16_16_BMP);
		m_TrayWebBmp.LoadBitmap(IDB_WEB_16_16_BMP);
		m_TrayClearBmp.LoadBitmap(IDB_CLEAR_16_16_BMP);

		m_TrayMenu.GetSubMenu(0)->SetMenuItemBitmaps(DS_TRAYMENU_SHOW, MF_BYCOMMAND, &m_TrayMainBmp, &m_TrayMainBmp);
		m_TrayMenu.GetSubMenu(0)->SetMenuItemBitmaps(DS_TRAYMENU_SYSSET, MF_BYCOMMAND, &m_TraySyssetBmp, &m_TraySyssetBmp);
		m_TrayMenu.GetSubMenu(0)->SetMenuItemBitmaps(ID_APP_EXIT, MF_BYCOMMAND, &m_TrayExitBmp, &m_TrayExitBmp);
		m_TrayMenu.GetSubMenu(0)->SetMenuItemBitmaps(DS_TRAYMENU_ABOUT, MF_BYCOMMAND, &m_TrayAboutBmp, &m_TrayAboutBmp);
		m_TrayMenu.GetSubMenu(0)->SetMenuItemBitmaps(DS_TRAYMENU_WEB, MF_BYCOMMAND, &m_TrayWebBmp, &m_TrayWebBmp);
		m_TrayMenu.GetSubMenu(0)->SetMenuItemBitmaps(DS_TRAYMENU_MEMFREE, MF_BYCOMMAND, &m_TrayClearBmp, &m_TrayClearBmp);

		/*��ʼ��Main menu*/
		m_MainMenu.LoadMenu(IDR_MAIN_MENU); //��ȡ��Դ
		m_MainListMenu.LoadMenu(IDR_MAIN_R_MENU); //��ȡ��Դ
		m_PrintRMenu.LoadMenu(IDR_PRINT_R_MENU); //��ȡ��Դ

		m_NormalMenu.LoadMenu(IDR_LIST_HEAD_NORMAL_MENU); //��ȡ��Դ
		m_DoubleMenu.LoadMenu(IDR_LIST_HEAD_DOUBLE_MENU); //��ȡ��Դ
		m_BlackMenu.LoadMenu(IDR_LIST_HEAD_BLACK_MENU); //��ȡ��Դ
	}
	catch(...)
	{
		nRet = RET_ERROR;
	}
	return nRet;
}

LRESULT CIPv6CloudDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	//TRACE("%d %d %d\n",message, wParam, lParam);
	switch(message)
	{
	case WM_NOTIFYICON: //������û��������Ϣ 
		if(lParam==WM_LBUTTONDBLCLK || lParam==WM_LBUTTONUP)
		{ 
			//���˫��ʱ�����ڳ���
			if(IsWindowVisible())
			{
				ShowWindow(SW_HIDE);
			}
			else
			{
				ShowWindow(SW_NORMAL);
			}
		}
		else if(lParam==WM_RBUTTONDOWN)
		{
			CPoint pos;
			GetCursorPos(&pos);

			/* ���õ�һ����ťΪ���� */
			//::SetMenuDefaultItem(m_TrayMenu->m_hMenu, 0, TRUE);

			/*���⵱�û�����ESCAPE�������ڲ˵�֮�ⵥ�����ʱ���˵�������ʧ��*/
			::SetForegroundWindow(m_NotifyIconData.hWnd);

			Sleep(50);

			/* �����λ�õ����˵� */
			m_TrayMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, AfxGetMainWnd());
		}
		break;
	case WM_SYSCOMMAND: //�����ϵͳ��Ϣ 
		if(wParam==SC_CLOSE)
		{ 
			//���յ���С����Ϣʱ���������� 
			ShowWindow(SW_HIDE);
		}
		else if(wParam==SC_RESTORE)
		{
			ShowWindow(SW_SHOWNORMAL);
		}
		else if(wParam==SC_MINIMIZE)
		{ 
			ShowWindow(SW_MINIMIZE);
		}
		break;
	case WM_AUTO_PRINT_MSG:
		{
			if(IDYES == AfxMessageBox("��δ��ɵ���ͨ�������Ƿ�ʼ�Զ���ӡ��", MB_YESNO|MB_ICONINFORMATION))
			{
				m_bAutoPrint = true;
				m_DuiMainDlg.m_btnPrintOff->SetVisible(false);
				m_DuiMainDlg.m_btnPrintOn->SetVisible(true);
				ShowTrayTips("ϵͳ��ʾ", "�Զ���ӡģʽ�ѿ�����");
			}
			else
			{
				m_bAutoPrint = false;
				m_DuiMainDlg.m_btnPrintOff->SetVisible(true);
				m_DuiMainDlg.m_btnPrintOn->SetVisible(false);
				ShowTrayTips("ϵͳ��ʾ", "�Զ���ӡģʽ�ѹرգ�");
			}
		}
	default:
		//TRACE("\n=========WindowProc========%d, %d\n", message, wParam);
		break;
	} 
	return CDialogEx::WindowProc(message, wParam, lParam);
}

/* ���̲˵� �ڴ����� */
void CIPv6CloudDlg::ToMemFree() 
{
	FreeMem();
}

void CIPv6CloudDlg::OnDestroy()
{
	SetLoginStatus(0);

	m_TCP.Close();

	//�ͷ������
	WSACleanup();

	CDialogEx::OnDestroy();

	// �����˳���ɾ�����½�����ͼ��
	Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
	//delete m_TrayMenu;
}

/* ���̲˵� ������ ��ʾ */
void CIPv6CloudDlg::OnTrayMenuShow()
{
	ShowWindow(SW_NORMAL);	
	SetForegroundWindow();
}

/* ���̲˵� �ٷ���վ */
void CIPv6CloudDlg::OnTrayMenuWeb()
{
	ShellExecute(NULL, "open", DS_TRAY_WEB_STR, NULL, NULL, SW_SHOWNORMAL);
}

/* ���̲˵� �ڴ����� */
void CIPv6CloudDlg::OnTrayMenuMemfree()
{
	ToMemFree();
	ShowTrayTips("ϵͳ��ʾ", "�ڴ�������ɣ�");
}

/* ���̲˵� �ڴ����� */
void CIPv6CloudDlg::OnTrayMenuSysset()
{
}

/* ���̲˵� �������� */
void CIPv6CloudDlg::OnTrayMenuAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// ��ʾ��ʾ��Ϣ
void CIPv6CloudDlg::ShowTrayTips(CString strTitle, CString strContent)
{
	_tcscpy(m_NotifyIconData.szInfoTitle, strTitle);
    _tcscpy(m_NotifyIconData.szInfo, strContent);
    m_NotifyIconData.uTimeout=1000; 
    m_NotifyIconData.uVersion=NOTIFYICON_VERSION; 
    Shell_NotifyIcon(NIM_MODIFY,&m_NotifyIconData);
}

//ѡ���ļ��жԻ���ص�����
int CALLBACK BrowseCallBackFun(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch(uMsg)
	{
	case BFFM_INITIALIZED:  //ѡ���ļ��жԻ����ʼ��
		//����Ĭ��·��ΪlpData��'D:\'
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		//��STATUSTEXT������ʾ��ǰ·��
		::SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, lpData);
		//����ѡ���ļ��жԻ���ı���
		::SetWindowText(hwnd, TEXT("�������ø�����Ŀ¼")); 
		break;
	case BFFM_SELCHANGED:   //ѡ���ļ��б��ʱ
		{
			TCHAR pszPath[MAX_PATH];
			//��ȡ��ǰѡ��·��
			SHGetPathFromIDList((LPCITEMIDLIST)lParam, pszPath);
			//��STATUSTEXT������ʾ��ǰ·��
			::SendMessage(hwnd, BFFM_SETSTATUSTEXT, TRUE, (LPARAM)pszPath);
		}
		break;
	}
	return 0;
}

// ѡ�񱣴�·��
void CIPv6CloudDlg::OpenPath()
{
	char szLocalFilePath[MAX_PATH] = {0};
	if(CGlobal::g_SavePath == NULL || strlen(CGlobal::g_SavePath) <= 0 || strcmp(CGlobal::g_SavePath, DEFAULT_DOWNLOAD_FILE_PATH) == 0)
	{
		sprintf(szLocalFilePath, "%s%s", CGlobal::g_ExePath, DEFAULT_DOWNLOAD_FILE_PATH);
	}
	else
	{
		sprintf(szLocalFilePath, "%s", CGlobal::g_SavePath);
	}

	ShellExecute( NULL, _T("open"), _T("explorer.exe"), szLocalFilePath, NULL, SW_SHOWNORMAL );
}

// ѡ�񱣴�·��
void CIPv6CloudDlg::PathBrowser()
{
	// TODO: Add your control notification handler code here
	TCHAR pszPath[MAX_PATH];
	BROWSEINFO bi; 
	bi.hwndOwner      = this->GetSafeHwnd();
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = NULL; 
	bi.lpszTitle      = TEXT("��ѡ���ļ���"); 
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
	bi.lpfn           = BrowseCallBackFun;     //�ص�����
	bi.lParam         = (LPARAM)TEXT("D:\\");  //�����ص������Ĳ���,����Ĭ��·��
	bi.iImage         = 0; 

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl == NULL)
	{
		return;
	}

	if (SHGetPathFromIDList(pidl, pszPath))
	{
		m_DuiMainDlg.SetSavePath(pszPath);
	}
}


// ���δ�������б�
int CIPv6CloudDlg::GetUnOrderList(int nPage)
{
	Json::Value data;  // data
	data["start"] = Json::Value(nPage * MAIN_ORDER_LIST_PAGE_NUM);
	data["limit"] = Json::Value(MAIN_ORDER_LIST_PAGE_NUM);

	Json::Value root;  // ��ʾ���� json ����
	root["data"] = data;
	root["userid"] = Json::Value(m_LoginDlg.m_szUserID);

	Json::FastWriter fast_writer;
	CString url = fast_writer.write(root).c_str();

	bool bret = false;
	if (g_pHttpJsonReader)
	{
		Json::Value value;
		Json::Value valArray;

		char strParam[1024] = {0};
		sprintf(strParam, "data=%s", url.GetBuffer(0));

		bool bSucceed = g_pHttpJsonReader->PostReadJson(m_LoginDlg.m_szServerAddr, "/clientapi/unhandled_orders", value, strParam);
		if (bSucceed)
		{
			m_nTotalUnOrder = value["total"].asInt();
			m_nSpecialUnOrder = value["special"].asInt();
			m_nNodeliveryUnOrder = value["nodelivery"].asInt();

			Json::Value valArray;
			valArray = value["data"];
			if (valArray.isArray())
			{
				int nSize = valArray.size();
				
				m_UnOrderListLock.Lock();
				for (UINT unIndex = 0; unIndex < nSize; unIndex++)
				{
					ST_ORDER stOrder = {0};

					strcpy(stOrder.orderId, valArray[unIndex]["orderId"].asCString());

					char *pdocName = NULL;
					CHttpJsonReader::UTF8Str2AnsiStr(valArray[unIndex]["docName"].asCString(), &pdocName);
					strcpy(stOrder.docName, pdocName);
					delete pdocName;

					stOrder.docId = atoi(valArray[unIndex]["docId"].asCString());
					strcpy(stOrder.userPhone, valArray[unIndex]["userPhone"].asCString());

					char *puserNick = NULL;
					CHttpJsonReader::UTF8Str2AnsiStr(valArray[unIndex]["userNick"].asCString(), &puserNick);
					strcpy(stOrder.userNick, puserNick);
					delete puserNick;

					char *puserName = NULL;
					CHttpJsonReader::UTF8Str2AnsiStr(valArray[unIndex]["userName"].asCString(), &puserName);
					strcpy(stOrder.userName, puserName);
					delete puserName;

					strcpy(stOrder.makeTime, valArray[unIndex]["makeTime"].asCString());
					strcpy(stOrder.upTime, valArray[unIndex]["upTime"].asCString());

					char *pshopName = NULL;
					CHttpJsonReader::UTF8Str2AnsiStr(valArray[unIndex]["shopName"].asCString(), &pshopName);
					strcpy(stOrder.shopName, pshopName);
					delete pshopName;

					stOrder.docFormat = atoi(valArray[unIndex]["docFormat"].asCString());
					stOrder.businessId = atoi(valArray[unIndex]["businessId"].asCString());

					char *pmessage = NULL;
					CHttpJsonReader::UTF8Str2AnsiStr(valArray[unIndex]["message"].asCString(), &pmessage);
					strcpy(stOrder.message, pmessage);
					delete pmessage;

					stOrder.price = atof(valArray[unIndex]["price"].asCString());
					stOrder.doublePrint = atoi(valArray[unIndex]["doublePrint"].asCString());
					stOrder.sendStatus = atoi(valArray[unIndex]["sendStatus"].asCString());
					stOrder.sendType = atoi(valArray[unIndex]["sendType"].asCString());
					stOrder.specialOrder = atoi(valArray[unIndex]["specialOrder"].asCString());
					stOrder.colorPrint = atoi(valArray[unIndex]["colorPrint"].asCString());
					stOrder.printCopis = atoi(valArray[unIndex]["printCopis"].asCString());
					stOrder.payStatus = atoi(valArray[unIndex]["payStatus"].asCString());
					stOrder.printStatus = atoi(valArray[unIndex]["printStatus"].asCString());

					stOrder.bHanding = false;

					m_UnOrderList.push_back(stOrder);
				}
				m_UnOrderListLock.UnLock();
			}
			else
			{
				ShowTrayTips("������ʾ", "���δ�������б��ظ�ʽ����");
				return -1;
			}
		}
		else
		{
			ShowTrayTips("������ʾ", "��ȡδ��ɶ���ʧ�ܣ�");
			return -1;
		}
	}
	else
	{
		ShowTrayTips("������ʾ", "��ȡδ��ɶ���ʧ������������ϣ�����������");
		return -1;
	}
	return m_nTotalUnOrder;
}

// ˢ��δ�������б�
void CIPv6CloudDlg::RefreshUnOrderList()
{
	char szTmp[32] = {0};
	sprintf(szTmp, "%d", m_nTotalUnOrder);
	m_DuiMainDlg.m_btnMainCount->SetText(szTmp);
	sprintf(szTmp, "%d", m_nSpecialUnOrder);
	m_DuiMainDlg.m_btnMainSpe->SetText(szTmp);
	sprintf(szTmp, "%d", m_nNodeliveryUnOrder);
	m_DuiMainDlg.m_labMainUn->SetText(szTmp);
	m_UnOrderListLock.Lock();
	int nSize = m_UnOrderList.size();
	m_DuiMainDlg.m_listMain->RemoveAll();
	int nShowIndex = 0;

	if(m_nCurrentPage < 0)
	{
		m_nCurrentPage = 0;
	}

	vector<ST_ORDER> UnList;
	UnList.clear();
	for (int i=0; i<nSize ; i++)
	{
		if(m_strMainSearch != "" && m_strMainSearch != "������绰����")
		{
			CString strSearch = "";
			strSearch.Format("%s", m_UnOrderList[i].userPhone);
			if(strSearch.Find(m_strMainSearch) >= 0)
			{
				UnList.push_back(m_UnOrderList[i]);
			}
		}
		else
		{
			//���ⶩ��
			UnList.push_back(m_UnOrderList[i]);
		}
	}

	vector<ST_ORDER>::iterator iter;
	for(iter = UnList.begin(); iter != UnList.end();)
	{
		ST_ORDER stUnO = *iter;
		if(m_nSelMainNormal != -1)
		{
			if(stUnO.specialOrder != m_nSelMainNormal)
			{
				iter = UnList.erase(iter);
				continue;
			}
		}

		if(m_nSelMainDouble != -1)
		{
			if(stUnO.doublePrint != m_nSelMainDouble)
			{
				iter = UnList.erase(iter);
				continue;
			}
		}

		if(m_nSelMainBlack != -1)
		{
			if(stUnO.colorPrint != m_nSelMainBlack)
			{
				iter = UnList.erase(iter);
				continue;
			}
		}

		iter++;
	}
	
	
	int nShowCount = 0;
	nSize = UnList.size();

	//������һҳû��������
	while(m_nCurrentPage>0 && m_nCurrentPage * MAIN_ORDER_LIST_PAGE_NUM >= nSize)
	{
		m_nCurrentPage--;
	}

	for (int i = m_nCurrentPage * MAIN_ORDER_LIST_PAGE_NUM; nShowIndex < MAIN_ORDER_LIST_PAGE_NUM && i<nSize ; i++)
	{
		m_DuiMainDlg.AddToMainList(i+1, UnList[i]);
		nShowIndex++;
	}
	m_UnOrderListLock.UnLock();
	
	sprintf(szTmp, "%d", m_nCurrentPage+1);
	m_DuiMainDlg.m_labMainCurPage->SetText(szTmp);
	int nTotalPage = ceil((float)nSize / MAIN_ORDER_LIST_PAGE_NUM);
	sprintf(szTmp, "%d", nTotalPage);
	m_DuiMainDlg.m_labMainTotalPage->SetText(szTmp);
}

// �����ʷ�����б�
int CIPv6CloudDlg::GetHisOrderList(int nPage)
{
	Json::Value data;  // data
	data["start"] = Json::Value(nPage * HIS_ORDER_LIST_PAGE_NUM);
	data["limit"] = Json::Value(HIS_ORDER_LIST_PAGE_NUM);

	Json::Value root;  // ��ʾ���� json ����
	root["data"] = data;
	root["userid"] = Json::Value(m_LoginDlg.m_szUserID);

	Json::FastWriter fast_writer;
	CString url = fast_writer.write(root).c_str();

	bool bret = false;
	if (g_pHttpJsonReader)
	{
		Json::Value value;
		Json::Value valArray;

		char strParam[1024] = {0};
		sprintf(strParam, "data=%s", url.GetBuffer(0));

		bool bSucceed = g_pHttpJsonReader->PostReadJson(m_LoginDlg.m_szServerAddr, "/clientapi/history_orders", value, strParam);
		if (bSucceed)
		{
			m_nHisTotalOrder = value["total"].asInt();

			Json::Value valArray;
			valArray = value["data"];
			if (valArray.isArray())
			{
				int nSize = valArray.size();
				
				m_HisOrderListLock.Lock();
				for (UINT unIndex = 0; unIndex < nSize; unIndex++)
				{
					ST_ORDER stOrder = {0};

					strcpy(stOrder.orderId, valArray[unIndex]["orderId"].asCString());

					char *pdocName = NULL;
					CHttpJsonReader::UTF8Str2AnsiStr(valArray[unIndex]["docName"].asCString(), &pdocName);
					strcpy(stOrder.docName, pdocName);
					delete pdocName;

					stOrder.docId = atoi(valArray[unIndex]["docId"].asCString());
					strcpy(stOrder.userPhone, valArray[unIndex]["userPhone"].asCString());

					char *puserNick = NULL;
					CHttpJsonReader::UTF8Str2AnsiStr(valArray[unIndex]["userNick"].asCString(), &puserNick);
					strcpy(stOrder.userNick, puserNick);
					delete puserNick;

					char *puserName = NULL;
					CHttpJsonReader::UTF8Str2AnsiStr(valArray[unIndex]["userName"].asCString(), &puserName);
					strcpy(stOrder.userName, puserName);
					delete puserName;

					strcpy(stOrder.makeTime, valArray[unIndex]["makeTime"].asCString());
					strcpy(stOrder.upTime, valArray[unIndex]["upTime"].asCString());

					char *pshopName = NULL;
					CHttpJsonReader::UTF8Str2AnsiStr(valArray[unIndex]["shopName"].asCString(), &pshopName);
					strcpy(stOrder.shopName, pshopName);
					delete pshopName;

					stOrder.docFormat = atoi(valArray[unIndex]["docFormat"].asCString());
					stOrder.businessId = atoi(valArray[unIndex]["businessId"].asCString());

					char *pmessage = NULL;
					CHttpJsonReader::UTF8Str2AnsiStr(valArray[unIndex]["message"].asCString(), &pmessage);
					strcpy(stOrder.message, pmessage);
					delete pmessage;

					stOrder.price = atof(valArray[unIndex]["price"].asCString());
					stOrder.doublePrint = atoi(valArray[unIndex]["doublePrint"].asCString());
					stOrder.sendStatus = atoi(valArray[unIndex]["sendStatus"].asCString());
					stOrder.sendType = atoi(valArray[unIndex]["sendType"].asCString());
					stOrder.specialOrder = atoi(valArray[unIndex]["specialOrder"].asCString());
					stOrder.colorPrint = atoi(valArray[unIndex]["colorPrint"].asCString());
					stOrder.printCopis = atoi(valArray[unIndex]["printCopis"].asCString());
					stOrder.payStatus = atoi(valArray[unIndex]["payStatus"].asCString());
					stOrder.printStatus = atoi(valArray[unIndex]["printStatus"].asCString());
					
					m_HisOrderList.push_back(stOrder);
				}
				m_HisOrderListLock.UnLock();
			}
			else
			{
				ShowTrayTips("������ʾ", "��ȡ��ʷ�������ظ�ʽ����");
				return -1;
			}
		}
		else
		{
			ShowTrayTips("������ʾ", "��ȡ��ʷ����ʧ�ܣ�");
			return -1;
		}
	}
	else
	{
		ShowTrayTips("������ʾ", "��ȡ��ʷ����ʧ������������ϣ�����������");
		return -1;
	}
	return m_nHisTotalOrder;
}

void Swap(ST_ORDER &data1, ST_ORDER &data2)
{
	ST_ORDER stTmp = {0};
	memcpy(&stTmp, &data1, sizeof(data1));
	memcpy(&data1, &data2, sizeof(data2));
	memcpy(&data2, &stTmp, sizeof(stTmp));
}

// ˢ����ʷ�����б�
void CIPv6CloudDlg::RefreshHisOrderList()
{
	char szTmp[32] = {0};

	m_HisOrderListLock.Lock();
	int nSize = m_HisOrderList.size();
	m_DuiMainDlg.m_listHistory->RemoveAll();
	int nShowIndex = 0;

	if(m_nHisCurrentPage < 0)
	{
		m_nHisCurrentPage = 0;
	}

	vector<ST_ORDER> HisList;
	HisList.clear();
	for (int i = 0; i<nSize ; i++)
	{
		if(m_strHisSearch != "" && m_strHisSearch != "������绰����")
		{
			CString strSearch = "";
			strSearch.Format("%s", m_HisOrderList[i].userPhone);
			if(strSearch.Find(m_strHisSearch) >= 0)
			{
				HisList.push_back(m_HisOrderList[i]);
			}
		}
		else
		{
			HisList.push_back(m_HisOrderList[i]);
		}
	}
	
	vector<ST_ORDER>::iterator iter;
	for(iter = HisList.begin(); iter != HisList.end();)
	{
		ST_ORDER stUnO = *iter;
		if(m_nSelHisNormal != -1)
		{
			if(stUnO.specialOrder != m_nSelHisNormal)
			{
				iter = HisList.erase(iter);
				continue;
			}
		}

		if(m_nSelHisDouble != -1)
		{
			if(stUnO.doublePrint != m_nSelHisDouble)
			{
				iter = HisList.erase(iter);
				continue;
			}
		}

		if(m_nSelHisBlack != -1)
		{
			if(stUnO.colorPrint != m_nSelHisBlack)
			{
				iter = HisList.erase(iter);
				continue;
			}
		}

		iter++;
	}

	nSize = HisList.size();
	int nShowCount = 0;
	//������һҳû��������
	while(m_nHisCurrentPage>0 && m_nHisCurrentPage * HIS_ORDER_LIST_PAGE_NUM >= nSize)
	{
		m_nHisCurrentPage--;
	}

	//for (int i = m_nHisCurrentPage * HIS_ORDER_LIST_PAGE_NUM; nShowIndex < HIS_ORDER_LIST_PAGE_NUM && i<nSize ; i++)
	//{
	//	m_DuiMainDlg.AddToHistoryList(i+1, HisList[i]);
	//	nShowIndex++;
	//}	

	/*����*/
	int lastSwapIndex = nSize - 1;
	int a, b;
    for (a = lastSwapIndex; a > 0;a = lastSwapIndex)
	{
		lastSwapIndex = 0;
		for (b = 0; b < a; b++)
		{
			CTime t1 = CDSUtils::CStringToCTime(HisList[b].makeTime);
			CTime t2 = CDSUtils::CStringToCTime(HisList[b + 1].makeTime);
			if (t1 < t2){
                Swap( HisList[b],HisList[b + 1]);
				lastSwapIndex = b;
			}
		}
	}

	for (int i = m_nHisCurrentPage * HIS_ORDER_LIST_PAGE_NUM; nShowIndex < HIS_ORDER_LIST_PAGE_NUM && i<nSize ; i++)
	{
		m_DuiMainDlg.AddToHistoryList(i+1, HisList[i]);
		nShowIndex++;
	}	
	m_HisOrderListLock.UnLock();
	
	sprintf(szTmp, "%d", m_nHisCurrentPage+1);
	m_DuiMainDlg.m_labHisCurPage->SetText(szTmp);
	int nTotalPage = ceil((float)nSize / HIS_ORDER_LIST_PAGE_NUM);
	sprintf(szTmp, "%d", nTotalPage);
	m_DuiMainDlg.m_labHisTotalPage->SetText(szTmp);
}

// ��ÿͻ��˵�ַ
void CIPv6CloudDlg::GetClientAddr()
{
	memset(m_szClientAddr, 0, sizeof(m_szClientAddr));
	Json::Value root;  // ��ʾ���� json ����
	root["userid"] = Json::Value(m_LoginDlg.m_szUserID);

	Json::FastWriter fast_writer;
	CString url = fast_writer.write(root).c_str();

	bool bret = false;
	if (g_pHttpJsonReader)
	{
		Json::Value value;
		Json::Value valArray;

		char strParam[1024] = {0};
		sprintf(strParam, "data=%s", url.GetBuffer(0));

		bool bSucceed = g_pHttpJsonReader->PostReadJson(m_LoginDlg.m_szServerAddr, "/clientapi/get_client_addr", value, strParam);
		if (bSucceed)
		{
			bool bRet = value["success"].asBool();

			if(bRet)
			{
				char *pclientAddr = NULL;
				CHttpJsonReader::UTF8Str2AnsiStr(value["clientAddr"].asCString(), &pclientAddr);
				strcpy(m_szClientAddr, pclientAddr);
				delete pclientAddr;
			}
		}
		else
		{
			ShowTrayTips("������ʾ", "��ÿͻ��˵�ַʱ���ӷ�����ʧ�ܣ�");
		}
	}
	else
	{
		ShowTrayTips("������ʾ", "��ÿͻ��˵�ַʧ������������ϣ�����������");
	}
}

// ����ļ����ص�ַ
CString CIPv6CloudDlg::GetDownloadFileUrl(ST_ORDER stOrder)
{
	CString strFileUrl = "";

	Json::Value data;  // data
	data["orderId"] = Json::Value(stOrder.orderId);

	Json::Value root;  // ��ʾ���� json ����
	root["data"] = data;
	root["userid"] = Json::Value(m_LoginDlg.m_szUserID);

	Json::FastWriter fast_writer;
	CString url = fast_writer.write(root).c_str();

	bool bret = false;
	if (g_pHttpJsonReader)
	{
		Json::Value value;
		Json::Value valArray;

		char strParam[1024] = {0};
		sprintf(strParam, "data=%s", url.GetBuffer(0));

		bool bSucceed = g_pHttpJsonReader->PostReadJson(m_LoginDlg.m_szServerAddr, "/clientapi/get_file_location", value, strParam);
		if (bSucceed)
		{
			char *pclientAddr = NULL;
			CHttpJsonReader::UTF8Str2AnsiStr(value["file_location"].asCString(), &pclientAddr);
			strFileUrl.Format("%s", pclientAddr);
			delete pclientAddr;
		}
		else
		{
			ShowTrayTips("������ʾ", "��GetDownloadFileUrl������ļ����ص�ַʱ���ӷ�����ʧ�ܣ�");
		}
	}
	else
	{
		ShowTrayTips("������ʾ", "����ļ����ص�ַʧ������������ϣ�����������");
	}
	return strFileUrl;
}

int CIPv6CloudDlg::SaveCfg(CString strLocalIpv6, CString strServerIpv6, CString strSavePath, CString strColorPrint, CString strBlackPrint)
{
	m_cGlobal.SaveCfg(CGlobal::g_ExePath, strLocalIpv6, strServerIpv6, strSavePath, strColorPrint, strBlackPrint);

	SetClientAddr(strLocalIpv6);
	ShowTrayTips("ϵͳ��ʾ", "�������óɹ���");
	return 0;
}

void CIPv6CloudDlg::SetClientAddr(CString strIPAddr)
{
	Json::Value data;  // data
	data["clientAddr"] = Json::Value(strIPAddr);
	data["port"] = Json::Value(CGlobal::g_ClientPort);

	Json::Value root;  // ��ʾ���� json ����
	root["data"] = data;
	root["userid"] = Json::Value(m_LoginDlg.m_szUserID);

	Json::FastWriter fast_writer;
	CString url = fast_writer.write(root).c_str();

	bool bret = true;
	if (g_pHttpJsonReader)
	{
		Json::Value value;
		Json::Value valArray;

		char strParam[1024] = {0};
		sprintf(strParam, "data=%s", url.GetBuffer(0));

		bool bSucceed = g_pHttpJsonReader->PostReadJson(m_LoginDlg.m_szServerAddr, "/clientapi/set_client_addr", value, strParam);
		if (bSucceed)
		{
			bool bRet = value["success"].asBool();

			if(bRet)
			{
			}
			else
			{
				char strErr[1024] = {0};
				char *pReason = NULL;
				CHttpJsonReader::UTF8Str2AnsiStr(value["reason"].asCString(), &pReason);
				strcpy(strErr, pReason);
				ShowTrayTips("������ʾ", pReason);
				delete pReason;
			}
		}
		else
		{
			ShowTrayTips("������ʾ", "���ÿͻ��˵�ַʧ�ܣ�");
		}
	}
	else
	{
		ShowTrayTips("������ʾ", "���ÿͻ��˵�ַ����������ϣ�����������");
	}
}

// ��ʷ���� һ������
CString CIPv6CloudDlg::ExportHis(CString strName, int nFlag)
{
	try
	{
		CString strExcelName = "";
		CTime tNow = CTime::GetCurrentTime();
		strExcelName.Format("��ʷ����_%d-%d-%d %02d%02d%02d", tNow.GetYear(), tNow.GetMonth(), tNow.GetDay(), tNow.GetHour(), tNow.GetMinute(), tNow.GetSecond());
		if(strName == "")
		{
			CFileDialog saveFile = CFileDialog(FALSE, ".xlsx", "", OFN_CREATEPROMPT | OFN_PATHMUSTEXIST , "Microsoft Office Excel ������ (*.xlsx)|*.xlsx|All files (*.*)|*.*||");
			CString defaultFileName = "";
			defaultFileName = strExcelName;
			saveFile.m_ofn.lpstrFile = defaultFileName.GetBuffer(MAX_PATH);
			saveFile.m_ofn.nMaxFile = MAX_PATH;
			if(IDOK == saveFile.DoModal()){
				strName = saveFile.GetPathName();
			}
			else
			{
				return "";
			}
		}

		CMyExcel ExcelMain;
		MyAlignment Xalign;
		MyNumberFormat Xnumber;
		MyBorder Xborder;
		MyFont Xfont;

		//�߿���
		Xborder.Color=RGB(0,0,0);
		Xborder.LineStyle=xlContinuous;
		Xborder.Weight=xlThin;
		//���뷽ʽ
		Xalign.HorizontalAlignment=xlCenter;
		Xalign.VerticalAlignment=xlTop;
		//��������
		Xnumber.GetText();
		//����
		Xfont.Bold=TRUE;
		Xfont.ForeColor=RGB(64,130,230);
		Xfont.Shadow=TRUE;
		Xfont.size=20;
		ExcelMain.Open();
		ExcelMain.AddSheet("��ʷ����");

		//���ñ���
		if(nFlag == 1)
		{
			//ExcelMain.GetRange("A1","K1");
			ExcelMain.GetRange("A1","J1");
		}
		else
		{
			ExcelMain.GetRange("A1","J1");
		}
		ExcelMain.SetMergeCells(TRUE);
		ExcelMain.SetAlignment(Xalign);
		ExcelMain.SetNumberFormat(Xnumber);
		ExcelMain.SetFont(Xfont);
		ExcelMain.SetItemText(1, 1, strExcelName);

		//���ñ�ͷ
		if(nFlag == 1)
		{
			//ExcelMain.GetRange("A2","K2");
			ExcelMain.GetRange("A2","J2");
		}
		else
		{
			ExcelMain.GetRange("A2","J2");
		}
		ExcelMain.SetAlignment(Xalign);
		ExcelMain.SetNumberFormat(Xnumber);
		ExcelMain.SetItemText(1, 1, "���");
		ExcelMain.SetItemText(1, 2, "���");
		ExcelMain.SetItemText(1, 3, "�µ�ʱ��");
		ExcelMain.SetItemText(1, 4, "�û���&�绰");
		ExcelMain.SetItemText(1, 5, "��ͨ");
		ExcelMain.SetItemText(1, 6, "��");
		ExcelMain.SetItemText(1, 7, "�ڰ�");
		ExcelMain.SetItemText(1, 8, "����");
		ExcelMain.SetItemText(1, 9, "�۸�");
		ExcelMain.SetItemText(1, 10, "״̬");
		if(nFlag == 1)
		{
			//ExcelMain.SetItemText(1, 11, "��ע");
		}

		m_HisOrderListLock.Lock();
		int nSize = m_HisOrderList.size();
		int nShowIndex = 0;

		//1 ��������
		if(nFlag == 1)
		{
			for (int i = 0; i<nSize ; i++)
			{
				CString strStart = "";
				CString strEnd = "";
				strStart.Format("A%d", nShowIndex + 3);
				strEnd.Format("K%d", nShowIndex + 3);
				ExcelMain.GetRange(strStart, strEnd);
				ExcelMain.SetAlignment(Xalign);
				ExcelMain.SetNumberFormat(Xnumber);
				CString strTmp = "";
				strTmp.Format("%d", nShowIndex + 1);
				ExcelMain.SetItemText(1, 1, strTmp);
				ExcelMain.SetItemText(1, 2, m_HisOrderList[i].orderId);
				ExcelMain.SetItemText(1, 3, m_HisOrderList[i].makeTime);
				strTmp.Format("%s&%s", m_HisOrderList[i].userName, m_HisOrderList[i].userPhone);
				ExcelMain.SetItemText(1, 4, strTmp);

				ExcelMain.SetItemText(1, 5, (m_HisOrderList[i].specialOrder > 0)?"����":"��ͨ");
				ExcelMain.SetItemText(1, 6, (m_HisOrderList[i].doublePrint > 0)?"˫��":"����");
				ExcelMain.SetItemText(1, 7, (m_HisOrderList[i].colorPrint > 0)?"��ɫ":"�ڰ�");
				strTmp.Format("%d", m_HisOrderList[i].printCopis);
				ExcelMain.SetItemText(1, 8, strTmp);
				strTmp.Format("%0.2f", m_HisOrderList[i].price);
				ExcelMain.SetItemText(1, 9, strTmp);
				ExcelMain.SetItemText(1, 10, (m_HisOrderList[i].payStatus > 0)?"��֧��":"δ֧��");
				//ExcelMain.SetItemText(1, 11, m_HisOrderList[i].message);

				nShowIndex++;
			}
		}
		else //0 ������ҳ
		{
			for (int i = m_nHisCurrentPage * HIS_ORDER_LIST_PAGE_NUM; nShowIndex < HIS_ORDER_LIST_PAGE_NUM && i<nSize ; i++)
			{
				CString strStart = "";
				CString strEnd = "";
				strStart.Format("A%d", nShowIndex + 3);
				strEnd.Format("K%d", nShowIndex + 3);
				ExcelMain.GetRange(strStart, strEnd);
				ExcelMain.SetAlignment(Xalign);
				ExcelMain.SetNumberFormat(Xnumber);
				CString strTmp = "";
				strTmp.Format("%d", nShowIndex + 1);
				ExcelMain.SetItemText(1, 1, strTmp);
				ExcelMain.SetItemText(1, 2, m_HisOrderList[i].orderId);
				ExcelMain.SetItemText(1, 3, m_HisOrderList[i].makeTime);
				strTmp.Format("%s&%s", m_HisOrderList[i].userName, m_HisOrderList[i].userPhone);
				ExcelMain.SetItemText(1, 4, strTmp);

				ExcelMain.SetItemText(1, 5, (m_HisOrderList[i].specialOrder > 0)?"����":"��ͨ");
				ExcelMain.SetItemText(1, 6, (m_HisOrderList[i].doublePrint > 0)?"˫��":"����");
				ExcelMain.SetItemText(1, 7, (m_HisOrderList[i].colorPrint > 0)?"��ɫ":"�ڰ�");
				strTmp.Format("%d", m_HisOrderList[i].printCopis);
				ExcelMain.SetItemText(1, 8, strTmp);
				strTmp.Format("%0.2f", m_HisOrderList[i].price);
				ExcelMain.SetItemText(1, 9, strTmp);
				ExcelMain.SetItemText(1, 10, (m_HisOrderList[i].payStatus > 0)?"��֧��":"δ֧��");
				//ExcelMain.SetItemText(1, 11, m_HisOrderList[i].message);

				nShowIndex++;
			}
		}
		m_HisOrderListLock.UnLock();

		ExcelMain.AutoRange();
		ExcelMain.AutoColFit();
		ExcelMain.SaveAs(strName);
	}
	catch(...)
	{
		ShowTrayTips("������ʾ", "������ʷ�����쳣��");
	}

	return strName;
}

// ���ÿͻ��˴�����״̬
bool CIPv6CloudDlg::SetOrderStatus(char *szOrderId, int nPayStatus, int nPrintStatus, int nSendStatus)
{
	Json::Value data;  // data
	data["orderId"] = Json::Value(szOrderId);
	data["printStatus"] = Json::Value(nPrintStatus);
	data["payStatus"] = Json::Value(nPayStatus);
	data["sendStatus"] = Json::Value(nSendStatus);

	Json::Value root;  // ��ʾ���� json ����
	root["data"] = data;
	root["userid"] = Json::Value(m_LoginDlg.m_szUserID);

	Json::FastWriter fast_writer;
	CString url = fast_writer.write(root).c_str();

	bool bret = true;
	if (g_pHttpJsonReader)
	{
		Json::Value value;
		Json::Value valArray;

		char strParam[1024] = {0};
		sprintf(strParam, "data=%s", url.GetBuffer(0));

		bool bSucceed = g_pHttpJsonReader->PostReadJson(m_LoginDlg.m_szServerAddr, "/clientapi/handled_orders", value, strParam);
		if (bSucceed)
		{
			bool bRet = value["success"].asBool();

			if(bRet)
			{
				return bRet;
			}
			else
			{
				char strErr[1024] = {0};
				char *pReason = NULL;
				CHttpJsonReader::UTF8Str2AnsiStr(value["reason"].asCString(), &pReason);
				strcpy(strErr, pReason);
				ShowTrayTips("������ʾ", pReason);
				delete pReason;
				return false;
			}
		}
		else
		{
			ShowTrayTips("������ʾ", "���ÿͻ��˴�����״̬ʧ�ܣ�");
			return false;
		}
	}
	else
	{
		ShowTrayTips("������ʾ", "���ÿͻ��˴�����״̬����������ϣ�����������");
		return false;
	}
	return true;
}
// ��ò������б�
void CIPv6CloudDlg::GetAndRefresh(int nFlag)
{
	if(nFlag == 0)
	{
		m_UnOrderListLock.Lock();
		m_UnOrderList.clear();
		m_UnOrderListLock.UnLock();
		GetUnOrderList(0);
		if(m_nTotalUnOrder > 0)
		{
			int nPageCount = m_nTotalUnOrder / MAIN_ORDER_LIST_PAGE_NUM;
			if(m_nTotalUnOrder % MAIN_ORDER_LIST_PAGE_NUM != 0)
			{
				nPageCount ++;
			}
			for(int i=1; i<nPageCount; i++)
			{
				GetUnOrderList(i);
			}
		}
		RefreshUnOrderList();
	}
	else
	{
		m_HisOrderListLock.Lock();
		m_HisOrderList.clear();
		m_HisOrderListLock.UnLock();
		GetHisOrderList(0);
		if(m_nHisTotalOrder > 0)
		{
			int nPageCount = m_nHisTotalOrder / HIS_ORDER_LIST_PAGE_NUM;
			if(m_nHisTotalOrder % HIS_ORDER_LIST_PAGE_NUM != 0)
			{
				nPageCount ++;
			}
			for(int i=1; i<nPageCount; i++)
			{
				GetHisOrderList(i);
			}
		}
		RefreshHisOrderList();
	}
}
// �Զ���ӡ
bool CIPv6CloudDlg::Print(CString strFileName, ST_ORDER stOrder)
{
	CString strPrintName = "";
	if(stOrder.colorPrint == 0)
	{
		int nCount = m_DuiMainDlg.m_listBlackPrint->GetCount();
		for(int i=0; i<nCount; i++)
		{
			CControlUI* pChildCtrl = m_DuiMainDlg.m_listBlackPrint->GetItemAt(i);
			if(pChildCtrl != NULL)
			{
				CControlUI *labName =m_DuiMainDlg.m_pm.FindSubControlByName(pChildCtrl, _T("labName"));
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
	}
	else
	{
		int nCount = m_DuiMainDlg.m_listColorPrint->GetCount();
		for(int i=0; i<nCount; i++)
		{
			CControlUI* pChildCtrl = m_DuiMainDlg.m_listColorPrint->GetItemAt(i);
			if(pChildCtrl != NULL)
			{
				CControlUI *labName =m_DuiMainDlg.m_pm.FindSubControlByName(pChildCtrl, _T("labName"));
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
	}

	if(strPrintName == "")
	{
		ShowTrayTips("������ʾ", "��ӡ����ʧ�ܣ���������һ̨��ӡ����");
		return false;
	}

	if(!SetSystemDefaultPrinter(strPrintName.GetBuffer(0)))
	{
		ShowTrayTips("������ʾ", "����Ĭ�ϴ�ӡ��ʧ�ܣ�");
		return false;
	}

	char szMsg[1024] = {0};
	sprintf(szMsg, "������ţ�%s �ļ�����%s", stOrder.orderId, stOrder.docName);
	if(StartPrint(strFileName.GetBuffer(0), stOrder.printCopis))
	{
		ShowTrayTips("��ӡ�ɹ���", szMsg);
	}
	else
	{
		ShowTrayTips("��ӡ�ļ�ʧ�ܣ�", szMsg);
	}
	
	/*
	switch(stOrder.docFormat)
	{
	case 1: //doc
		//{
		//	return CDSPrint::PrintWord(strPrintName, strFileName, stOrder.printCopis);
		//}
		//break;
	case 2: //pdf
	case 3: //docx
	case 4: //pdfx
	case 5: //xlsx
	case 6: //ppt
	case 7: //pptx
	case 8: //wps
	case 9: //gif
	case 10: //jpg
	case 11: //png
	case 12: //xls
	case 13: //jpeg
		{
			//extern void Demo(CString strPrintName, CString strFileName);
			//Demo(strPrintName, strFileName);
			char szMsg[1024] = {0};
			sprintf(szMsg, "������ţ�%s �ļ�����%s", stOrder.orderId, stOrder.docName);
			if(StartPrint(strFileName.GetBuffer(0), stOrder.printCopis))
			{
				ShowTrayTips("��ӡ�ɹ���", szMsg);
			}
			else
			{
				ShowTrayTips("��ӡ�ļ�ʧ�ܣ�", szMsg);
			}
		}
		break;
	}
	*/

	return true;
}

// ��ӡ��ʷ����
void CIPv6CloudDlg::PrintHis()
{
	_beginthread(PrintHisSvc, NULL, this);
}

// ����δ������������״̬
void  CIPv6CloudDlg::UpdateUnOrderListStatus(FileItem * pMFI, bool bStatus, bool bHanding)
{
	m_UnOrderListLock.Lock();
	int nSize = m_UnOrderList.size();
	for(int i=0; i<nSize; i++)
	{
		if(strcmp(pMFI->orderId, m_UnOrderList[i].orderId) == 0)
		{
			if(bStatus)
			{
				if(pMFI->bPrint)
				{
					//ֱ�Ӵ�ӡ
					if(Print(pMFI->szLocalFilePath, m_UnOrderList[i]))
					{
						SetOrderStatus(m_UnOrderList[i].orderId, m_UnOrderList[i].payStatus, 1, m_UnOrderList[i].sendStatus);
						GetAndRefresh(0);
						GetAndRefresh(1);
					}
				}
			}

			m_UnOrderList[i].bHanding = false;
			break;
		}
	}
	m_UnOrderListLock.UnLock();

	CString strMsg = "";
	strMsg.Format("��ţ�%s����ַ��%s", pMFI->orderId, pMFI->szLocalFilePath);
	if(bStatus)
	{
		if(pMFI->bPrint)
		{
			//ShowTrayTips("��ӡ���", strMsg);
		}
		else
		{
			ShowTrayTips("�������", strMsg);
		}
	}
	else
	{
		if(pMFI->bPrint)
		{
			//ShowTrayTips("��ӡʧ��", strMsg);
		}
		else
		{
			ShowTrayTips("����ʧ��", strMsg);
		}
	}
}

// ���� ���״̬Ϊ �˵�
void CIPv6CloudDlg::OnSetStatusBtn()
{
	m_nMainMenuFlag = 0;

	CPoint pos;
	GetCursorPos(&pos);

	Sleep(50);

	CString strIDList = GetListOrderVec();
	if(strIDList.Find("][") > 0)
	{
		m_MainMenu.GetSubMenu(0)->ModifyMenu(0, MF_BYPOSITION, ID_MAIN_DOWNS, "��������");
		m_MainMenu.GetSubMenu(0)->ModifyMenu(1, MF_BYPOSITION, ID_MAIN_PRINTS, "������ӡ");
	}
	else
	{
		m_MainMenu.GetSubMenu(0)->ModifyMenu(0, MF_BYPOSITION, ID_MAIN_DOWNS, "��������");
		m_MainMenu.GetSubMenu(0)->ModifyMenu(1, MF_BYPOSITION, ID_MAIN_PRINTS, "������ӡ");
	}
	/* �����λ�õ����˵� */
	m_MainMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, AfxGetMainWnd());
}

// ���� ҵ����/��ʷ�����Ҽ��˵�
void  CIPv6CloudDlg::MainRightDown(int nFlag)
{
	m_nMainMenuFlag = nFlag;
	CPoint pos;
	GetCursorPos(&pos);

	Sleep(50);

	// ��ѡ��ǰ������
	if(m_nMainMenuFlag == 0)
	{
		int iIndex = m_DuiMainDlg.m_listMain->GetCurSel();
		if(iIndex < 0)
		{
			return;
		}
		CControlUI *pCtrlEle = m_DuiMainDlg.m_listMain->GetItemAt(iIndex);
		if(pCtrlEle != NULL)
		{
			CControlUI *pCtrlSeled =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnSeled"));
			if(pCtrlSeled != NULL)
			{
				pCtrlSeled->SetVisible(true);
			}

			CControlUI *pCtrlUnSel =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnUnSel"));
			if(pCtrlUnSel != NULL)
			{
				pCtrlUnSel->SetVisible(false);
			}
		}
	}
	else if(m_nMainMenuFlag == 1)
	{
		int iIndex = m_DuiMainDlg.m_listHistory->GetCurSel();
		if(iIndex < 0)
		{
			return;
		}
		CControlUI *pCtrlEle = m_DuiMainDlg.m_listHistory->GetItemAt(iIndex);
		if(pCtrlEle != NULL)
		{
			CControlUI *pCtrlSeled =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnSeled"));
			if(pCtrlSeled != NULL)
			{
				pCtrlSeled->SetVisible(true);
			}

			CControlUI *pCtrlUnSel =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnUnSel"));
			if(pCtrlUnSel != NULL)
			{
				pCtrlUnSel->SetVisible(false);
			}
		}
	}

	CString strIDList = GetListOrderVec();
	if(strIDList.Find("][") > 0)
	{
		m_MainListMenu.GetSubMenu(0)->ModifyMenu(0, MF_BYPOSITION, ID_MIAN_R_DOWN, "��������");
		m_MainListMenu.GetSubMenu(0)->ModifyMenu(1, MF_BYPOSITION, ID_MIAN_R_PRINT, "������ӡ");
	}
	else
	{
		m_MainListMenu.GetSubMenu(0)->ModifyMenu(0, MF_BYPOSITION, ID_MIAN_R_DOWN, "��������");
		m_MainListMenu.GetSubMenu(0)->ModifyMenu(1, MF_BYPOSITION, ID_MIAN_R_PRINT, "������ӡ");
	}
	/* �����λ�õ����˵� */
	m_MainListMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, AfxGetMainWnd());
}

// ҵ�������������˵�
// ��õ�ǰ��ѡ����б��� �������
CString CIPv6CloudDlg::GetListOrderVec()
{
	CString strIDList = "";
	if(m_nMainMenuFlag == 0)
	{
		int nCount = m_DuiMainDlg.m_listMain->GetCount();
		for(int i=0; i<nCount; i++)
		{
			CControlUI* pCtrlEle = m_DuiMainDlg.m_listMain->GetItemAt(i);
			if(pCtrlEle != NULL)
			{
				CControlUI *pCtrlSeled =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnSeled"));
				if(pCtrlSeled != NULL)
				{
					if(pCtrlSeled->IsVisible())
					{
						CControlUI *pCtrl =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("labOrderId"));
						if(pCtrl != NULL)
						{
							CString strID = "";
							CString strOrderId = pCtrl->GetText();
							strID.Format("[%s]", strOrderId);
							strIDList += strID;
						}
					}
				}
			}
		}
	}
	else
	{
		int nCount = m_DuiMainDlg.m_listHistory->GetCount();
		for(int i=0; i<nCount; i++)
		{
			CControlUI* pCtrlEle = m_DuiMainDlg.m_listHistory->GetItemAt(i);
			if(pCtrlEle != NULL)
			{
				CControlUI *pCtrlSeled =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnSeled"));
				if(pCtrlSeled != NULL)
				{
					if(pCtrlSeled->IsVisible())
					{
						CControlUI *pCtrl =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("labOrderId"));
						if(pCtrl != NULL)
						{
							CString strID = "";
							CString strOrderId = pCtrl->GetText();
							strID.Format("[%s]", strOrderId);
							strIDList += strID;
						}
					}
				}
			}
		}
	}
	return  strIDList;
}

void CIPv6CloudDlg::OnMainDowns()
{
	CString strIDList = GetListOrderVec();
	if(m_nMainMenuFlag == 0)
	{
		m_UnOrderListLock.Lock();
		int nSize = m_UnOrderList.size();
		for (int i = 0; i<nSize ; i++)
		{
			CString strOidstr = "";
			strOidstr.Format("[%s]", m_UnOrderList[i].orderId);
			if(strIDList.Find(strOidstr) >= 0)
			{
				RDown(m_UnOrderList[i], false);
			}
		}
		m_UnOrderListLock.UnLock();
	}
	else
	{
		m_HisOrderListLock.Lock();
		int nSize = m_HisOrderList.size();
		for (int i = 0; i<nSize ; i++)
		{
			CString strOidstr = "";
			strOidstr.Format("[%s]", m_HisOrderList[i].orderId);
			if(strIDList.Find(strOidstr) >= 0)
			{
				RDown(m_HisOrderList[i], false);
			}
		}
		m_HisOrderListLock.UnLock();
	}
}

void CIPv6CloudDlg::OnMainPrints()
{
	CString strIDList = GetListOrderVec();
	if(m_nMainMenuFlag == 0)
	{
		m_UnOrderListLock.Lock();
		int nSize = m_UnOrderList.size();
		for (int i = 0; i<nSize ; i++)
		{
			CString strOidstr = "";
			strOidstr.Format("[%s]", m_UnOrderList[i].orderId);
			if(strIDList.Find(strOidstr) >= 0)
			{
				RPrint(m_UnOrderList[i]);
			}
		}
		m_UnOrderListLock.UnLock();
	}
	else
	{
		m_HisOrderListLock.Lock();
		int nSize = m_HisOrderList.size();
		for (int i = 0; i<nSize ; i++)
		{
			CString strOidstr = "";
			strOidstr.Format("[%s]", m_HisOrderList[i].orderId);
			if(strIDList.Find(strOidstr) >= 0)
			{
				RPrint(m_HisOrderList[i]);
			}
		}
		m_HisOrderListLock.UnLock();
	}
}

void CIPv6CloudDlg::OnMainSetstatus1()
{
	CString strIDList = GetListOrderVec();
	if(m_nMainMenuFlag == 0)
	{
		m_UnOrderListLock.Lock();
		int nSize = m_UnOrderList.size();
		for (int i = 0; i<nSize ; i++)
		{
			if(strIDList.Find(m_UnOrderList[i].orderId) >= 0)
			{
				RSetstatus1(m_UnOrderList[i]);
			}
		}
		m_UnOrderListLock.UnLock();
	}
	else
	{
		m_HisOrderListLock.Lock();
		int nSize = m_HisOrderList.size();
		for (int i = 0; i<nSize ; i++)
		{
			if(strIDList.Find(m_HisOrderList[i].orderId) >= 0)
			{
				RSetstatus1(m_HisOrderList[i]);
			}
		}
		m_HisOrderListLock.UnLock();
	}
	GetAndRefresh(0);
	GetAndRefresh(1);
}

void CIPv6CloudDlg::OnMainSetstatus2()
{
	CString strIDList = GetListOrderVec();
	if(m_nMainMenuFlag == 0)
	{
		m_UnOrderListLock.Lock();
		int nSize = m_UnOrderList.size();
		for (int i = 0; i<nSize ; i++)
		{
			if(strIDList.Find(m_UnOrderList[i].orderId) >= 0)
			{
				RSetstatus2(m_UnOrderList[i]);
			}
		}
		m_UnOrderListLock.UnLock();
	}
	else
	{
		m_HisOrderListLock.Lock();
		int nSize = m_HisOrderList.size();
		for (int i = 0; i<nSize ; i++)
		{
			if(strIDList.Find(m_HisOrderList[i].orderId) >= 0)
			{
				RSetstatus2(m_HisOrderList[i]);
			}
		}
		m_HisOrderListLock.UnLock();
	}
	GetAndRefresh(0);
	GetAndRefresh(1);
}

/* ҵ�����б��Ҽ��˵� */
// ��õ�ǰ��ѡ����б��� �������
void CIPv6CloudDlg::GetListOrderST(ST_ORDER &stOrder)
{
	CString strOrderId = "";
	if(m_nMainMenuFlag == 0)
	{
		int iIndex = m_DuiMainDlg.m_listMain->GetCurSel();
		if(iIndex < 0)
		{
			return;
		}
		CControlUI *pCtrlEle = m_DuiMainDlg.m_listMain->GetItemAt(iIndex);
		if(pCtrlEle != NULL)
		{
			CControlUI *pCtrl =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("labOrderId"));
			if(pCtrl != NULL)
			{
				strOrderId = pCtrl->GetText();
			}
		}
	}
	else if(m_nMainMenuFlag == 1)
	{
		int iIndex = m_DuiMainDlg.m_listHistory->GetCurSel();
		if(iIndex < 0)
		{
			return;
		}
		CControlUI *pCtrlEle = m_DuiMainDlg.m_listHistory->GetItemAt(iIndex);
		if(pCtrlEle != NULL)
		{
			CControlUI *pCtrl =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("labOrderId"));
			if(pCtrl != NULL)
			{
				strOrderId = pCtrl->GetText();
			}
		}
	}

	if(strOrderId != "")
	{
		if(m_nMainMenuFlag == 0)
		{
			m_UnOrderListLock.Lock();
			int nSize = m_UnOrderList.size();
			for (int i = 0; i<nSize ; i++)
			{
				if(strcmp(m_UnOrderList[i].orderId, strOrderId.GetBuffer(0)) == 0)
				{
					memcpy(&stOrder, &m_UnOrderList[i], sizeof(m_UnOrderList[i]));
					break;
				}
			}
			m_UnOrderListLock.UnLock();
		}
		else if(m_nMainMenuFlag == 1)
		{
			m_HisOrderListLock.Lock();
			int nSize = m_HisOrderList.size();
			for (int i = 0; i<nSize ; i++)
			{
				if(strcmp(m_HisOrderList[i].orderId, strOrderId.GetBuffer(0)) == 0)
				{
					memcpy(&stOrder, &m_HisOrderList[i], sizeof(m_UnOrderList[i]));
					break;
				}
			}
			m_HisOrderListLock.UnLock();
		}
	}
}

void CIPv6CloudDlg::OnMianRDown()
{
	OnMainDowns();
}

void CIPv6CloudDlg::OnMianRPrint()
{
	OnMainPrints();
}

void CIPv6CloudDlg::OnMainRSetstatus1()
{
	OnMainSetstatus1();
}

void CIPv6CloudDlg::OnMainRSetstatus2()
{
	OnMainSetstatus2();
}

/////////////////////////////////////////////////////////��ӡ���б��Ҽ��˵�///////////////////////////////////////////////////////////
// ��ӡ���б��Ҽ��˵�
void  CIPv6CloudDlg::PrintRDown(int nFlag)
{
	m_nPrintMenuFlag = nFlag;

	CPoint pos;
	GetCursorPos(&pos);

	Sleep(50);

	if(nFlag == 1)
	{
		m_PrintRMenu.GetSubMenu(0)->EnableMenuItem(0, MF_BYPOSITION|MF_ENABLED);
		m_PrintRMenu.GetSubMenu(0)->EnableMenuItem(1, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
	}
	else
	{
		m_PrintRMenu.GetSubMenu(0)->EnableMenuItem(0, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
		m_PrintRMenu.GetSubMenu(0)->EnableMenuItem(1, MF_BYPOSITION|MF_ENABLED);
	}

	/* �����λ�õ����˵� */
	m_PrintRMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, AfxGetMainWnd());
}

void CIPv6CloudDlg::RDown(ST_ORDER stOrder, bool bPath)
{
	CString strDownUrl = GetDownloadFileUrl(stOrder);
	if(strDownUrl == "")
	{
		ShowTrayTips("������ʾ", "��RDown������ļ����ص�ַʧ�ܣ����Ժ����ԣ�");
		return;
	}
	
	char szLocalFilePath[MAX_PATH] = {0};
	if(!bPath)
	{
		if(CGlobal::g_SavePath == NULL || strlen(CGlobal::g_SavePath) <= 0 || strcmp(CGlobal::g_SavePath, DEFAULT_DOWNLOAD_FILE_PATH) == 0)
		{
			sprintf(szLocalFilePath, "%s%s\\[%s]%s", CGlobal::g_ExePath, DEFAULT_DOWNLOAD_FILE_PATH, stOrder.orderId, stOrder.docName);
		}
		else
		{
			sprintf(szLocalFilePath, "%s\\[%s]%s", CGlobal::g_SavePath, stOrder.orderId, stOrder.docName);
		}
	}
	else
	{
		//��ú�׺��
		string strBackName(strDownUrl);
		int nLast=strBackName.find_last_of(".");
		strBackName=strBackName.substr(nLast+1);

		CString strFilter = "";
		strFilter.Format("�����ļ� (*%s)|*%s|All files (*.*)|*.*||", strBackName.c_str(), strBackName.c_str());

		//����ļ���
		CFileDialog saveFile = CFileDialog(FALSE, strBackName.c_str(), "", OFN_CREATEPROMPT | OFN_PATHMUSTEXIST , strFilter);
		CString defaultFileName = "";
		defaultFileName.Format("[%s]%s", stOrder.orderId, stOrder.docName);  
		saveFile.m_ofn.lpstrFile = defaultFileName.GetBuffer(MAX_PATH);  
		saveFile.m_ofn.nMaxFile = MAX_PATH;
		if(IDOK == saveFile.DoModal()){
			strcpy(szLocalFilePath, saveFile.GetPathName());
		}
		else
		{
			return;
		}
	}

	FileItem * stMFI = new FileItem;
	stMFI->bDeleteOnDownFinish = true;
	stMFI->pParent = this;
	stMFI->bLocal = false;
	stMFI->bPrint = false;
	strcpy(stMFI->orderId, stOrder.orderId);
	stMFI->nDocId = stOrder.docId;
	strcpy(stMFI->strSvrUrl, strDownUrl.GetBuffer(0));
	strcpy(stMFI->szLocalFilePath, szLocalFilePath);
	m_FileDownload.AddDownloadMediaFileItem(stMFI);
}

void CIPv6CloudDlg::RPrint(ST_ORDER stOrder)
{
	CString strDownUrl = GetDownloadFileUrl(stOrder);
	if(strDownUrl == "")
	{
		ShowTrayTips("������ʾ", "��RPrint������ļ����ص�ַʧ�ܣ����Ժ����ԣ�");
		return;
	}

	char szLocalFilePath[MAX_PATH] = {0};
	if(CGlobal::g_SavePath == NULL || strlen(CGlobal::g_SavePath) <= 0 || strcmp(CGlobal::g_SavePath, DEFAULT_DOWNLOAD_FILE_PATH) == 0)
	{
		sprintf(szLocalFilePath, "%s%s\\[%s]%s", CGlobal::g_ExePath, DEFAULT_DOWNLOAD_FILE_PATH, stOrder.orderId, stOrder.docName);
	}
	else
	{
		sprintf(szLocalFilePath, "%s\\[%s]%s", CGlobal::g_SavePath, stOrder.orderId, stOrder.docName);
	}

	// ���ش���
	if(access(szLocalFilePath, 0) == 0)
	{
		//ֱ�Ӵ�ӡ
		if(Print(szLocalFilePath, stOrder))
		{
			SetOrderStatus(stOrder.orderId, stOrder.payStatus, 1, stOrder.sendStatus);
			GetAndRefresh(0);
			GetAndRefresh(1);
		}
	}
	else
	{
		//�����ڣ����غ��ٴ�ӡ
		FileItem * stMFI = new FileItem;
		stMFI->bDeleteOnDownFinish = true;
		stMFI->pParent = this;
		stMFI->bLocal = false;
		stMFI->bPrint = true;
		strcpy(stMFI->orderId, stOrder.orderId);
		stMFI->nDocId = stOrder.docId;
		strcpy(stMFI->strSvrUrl, strDownUrl.GetBuffer(0));
		strcpy(stMFI->szLocalFilePath, szLocalFilePath);
		m_FileDownload.AddDownloadMediaFileItem(stMFI);
	}
}

void CIPv6CloudDlg::RSetstatus1(ST_ORDER stOrder)
{
	SetOrderStatus(stOrder.orderId, stOrder.payStatus, 1, stOrder.sendStatus);
}

void CIPv6CloudDlg::RSetstatus2(ST_ORDER stOrder)
{
	SetOrderStatus(stOrder.orderId, stOrder.payStatus, 0, stOrder.sendStatus);
}

/* ��ӡ���б��Ҽ��˵� */
void CIPv6CloudDlg::OnPrintRAddtoB()
{
	m_DuiMainDlg.AddBlackPrint();
}

void CIPv6CloudDlg::OnPrintRAddtoC()
{
	m_DuiMainDlg.AddColorPrint();
}

void CIPv6CloudDlg::OnPrintRDelete()
{
	if(m_nPrintMenuFlag == 0)
	{
		m_DuiMainDlg.DeleteBlackPrint();
	}
	else if(m_nPrintMenuFlag == 2)
	{
		m_DuiMainDlg.DeleteColorPrint();
	}
}

/* ��ҳ */
void CIPv6CloudDlg::Refresh(int nFlag)
{
	if(nFlag == 0)
	{
		m_nSelMainNormal = -1;
		m_nSelMainDouble = -1;
		m_nSelMainBlack = -1;
	}
	else
	{
		m_nSelHisNormal = -1;
		m_nSelHisDouble = -1;
		m_nSelHisBlack = -1;
	}

	GetAndRefresh(nFlag);
}

void CIPv6CloudDlg::SelAll(int nFlag)
{
	if(nFlag == 0)
	{
		int nCount = m_DuiMainDlg.m_listMain->GetCount();

		bool bHasUnSeled = false;

		for(int i=0; i<nCount; i++)
		{
			CControlUI* pCtrlEle = m_DuiMainDlg.m_listMain->GetItemAt(i);
			if(pCtrlEle != NULL)
			{
				CControlUI *pCtrlSeled =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnSeled"));
				if(pCtrlSeled != NULL)
				{
					if(!pCtrlSeled->IsVisible())
					{
						bHasUnSeled = true;
						break;
					}
				}
			}
		}

		for(int i=0; i<nCount; i++)
		{
			CControlUI* pCtrlEle = m_DuiMainDlg.m_listMain->GetItemAt(i);
			if(pCtrlEle != NULL)
			{
				if(bHasUnSeled)
				{
					CControlUI *pCtrlSeled =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnSeled"));
					if(pCtrlSeled != NULL)
					{
						pCtrlSeled->SetVisible(true);
					}

					CControlUI *pCtrlUnSel =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnUnSel"));
					if(pCtrlSeled != NULL)
					{
						pCtrlUnSel->SetVisible(false);
					}
				}
				else
				{
					CControlUI *pCtrlSeled =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnSeled"));
					if(pCtrlSeled != NULL)
					{
						pCtrlSeled->SetVisible(false);
					}

					CControlUI *pCtrlUnSel =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnUnSel"));
					if(pCtrlSeled != NULL)
					{
						pCtrlUnSel->SetVisible(true);
					}
				}
			}
		}
	}
	else
	{
		int nCount = m_DuiMainDlg.m_listHistory->GetCount();
		
		bool bHasUnSeled = false;

		for(int i=0; i<nCount; i++)
		{
			CControlUI* pCtrlEle = m_DuiMainDlg.m_listHistory->GetItemAt(i);
			if(pCtrlEle != NULL)
			{
				CControlUI *pCtrlSeled =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnSeled"));
				if(pCtrlSeled != NULL)
				{
					if(!pCtrlSeled->IsVisible())
					{
						bHasUnSeled = true;
						break;
					}
				}
			}
		}

		for(int i=0; i<nCount; i++)
		{
			CControlUI* pCtrlEle = m_DuiMainDlg.m_listHistory->GetItemAt(i);
			if(pCtrlEle != NULL)
			{
				if(bHasUnSeled)
				{
					CControlUI *pCtrlSeled =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnSeled"));
					if(pCtrlSeled != NULL)
					{
						pCtrlSeled->SetVisible(true);
					}

					CControlUI *pCtrlUnSel =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnUnSel"));
					if(pCtrlSeled != NULL)
					{
						pCtrlUnSel->SetVisible(false);
					}
				}
				else
				{
					CControlUI *pCtrlSeled =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnSeled"));
					if(pCtrlSeled != NULL)
					{
						pCtrlSeled->SetVisible(false);
					}

					CControlUI *pCtrlUnSel =m_DuiMainDlg.m_pm.FindSubControlByName(pCtrlEle, _T("btnUnSel"));
					if(pCtrlSeled != NULL)
					{
						pCtrlUnSel->SetVisible(true);
					}
				}
			}
		}
	}
}

void CIPv6CloudDlg::PrePage(int nFlag)
{
	if(nFlag == 0)
	{
		m_nCurrentPage--;
		RefreshUnOrderList();
	}
	else
	{
		m_nHisCurrentPage--;
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::NextPage(int nFlag)
{
	if(nFlag == 0)
	{
		m_nCurrentPage++;
		RefreshUnOrderList();
	}
	else
	{
		m_nHisCurrentPage++;
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::GotoPage(int nFlag, int nPage)
{
	if(nPage < 1)
	{
		nPage = 1;
	}

	if(nFlag == 0)
	{
		m_nCurrentPage = nPage - 1;
		RefreshUnOrderList();
	}
	else
	{
		m_nHisCurrentPage = nPage - 1;
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::Search(int nIndex, CString strSearch)
{
	if(nIndex == 0)
	{
		m_nCurrentPage = 0;
		m_strMainSearch = strSearch;
		RefreshUnOrderList();
	}
	else if(nIndex == 2)
	{
		m_nHisCurrentPage = 0;
		m_strHisSearch = strSearch;
		RefreshHisOrderList();
	}
}

// �������
void CIPv6CloudDlg::GetAccount()
{
	//CLabelUI *  m_labAccount1;
	//CLabelUI *  btnColorCount;
	//CLabelUI *  btnSingleCount;
	//CLabelUI *  m_btnOrderCount;
	//CLabelUI *  btnPaperCount;
	//CLabelUI *  btnBlackCount;
	//CLabelUI *  btnDoubleCount;
	//CLabelUI *  m_labAccountAddr;

	int nOrderCount = 0; //�����ն���
	int nPaperCount = 0; //������ֽ��
	int nColorCount = 0; //��ɫ��ӡ
	int nBlackCount = 0; //�ڰ״�ӡ
	int nSingleCount = 0; //�����ӡ
	int nDoubleCount = 0; //˫���ӡ

	CTime tNow = CTime::GetCurrentTime();
	CTime tToday(tNow.GetYear(), tNow.GetMonth(), tNow.GetDay(), 0, 0, 0);

	m_UnOrderListLock.Lock();
	int nSize = m_UnOrderList.size();
	for(int i=0; i<nSize; i++)
	{
		CTime tTime = CDSUtils::CStringToCTime(m_UnOrderList[i].makeTime);
		if(tTime > tToday)
		{
			nOrderCount ++;
			if(m_UnOrderList[i].printStatus == 1)
			{
				nPaperCount += m_UnOrderList[i].printCopis;

				if(m_UnOrderList[i].colorPrint == 1)
				{
					nColorCount++;
				}
				else
				{
					nBlackCount++;
				}

				if(m_UnOrderList[i].doublePrint == 1)
				{
					nDoubleCount++;
				}
				else
				{
					nSingleCount++;
				}
			}
		}
	}
	m_UnOrderListLock.UnLock();

	m_HisOrderListLock.Lock();
	nSize = m_HisOrderList.size();
	for(int i=0; i<nSize; i++)
	{
		CTime tTime = CDSUtils::CStringToCTime(m_HisOrderList[i].makeTime);
		if(tTime > tToday)
		{
			nOrderCount ++;
			if(m_HisOrderList[i].printStatus == 1)
			{
				nPaperCount += m_HisOrderList[i].printCopis;

				if(m_HisOrderList[i].colorPrint == 1)
				{
					nColorCount++;
				}
				else
				{
					nBlackCount++;
				}

				if(m_HisOrderList[i].doublePrint == 1)
				{
					nDoubleCount++;
				}
				else
				{
					nSingleCount++;
				}
			}
		}
	}
	m_HisOrderListLock.UnLock();
	
	CString strOrderCount = "";
	CString strPaperCount = "";
	CString strColorCount = "";
	CString strBlackCount = "";
	CString strSingleCount = "";
	CString strDoubleCount = "";

	strOrderCount.Format("%d", nOrderCount);
	strPaperCount.Format("%d", nPaperCount);
	strColorCount.Format("%d", nColorCount);
	strBlackCount.Format("%d", nBlackCount);
	strSingleCount.Format("%d", nSingleCount);
	strDoubleCount.Format("%d", nDoubleCount);

	m_DuiMainDlg.m_btnOrderCount->SetText(strOrderCount);
	m_DuiMainDlg.btnPaperCount->SetText(strPaperCount);
	m_DuiMainDlg.btnColorCount->SetText(strColorCount);
	m_DuiMainDlg.btnBlackCount->SetText(strBlackCount);
	m_DuiMainDlg.btnSingleCount->SetText(strSingleCount);
	m_DuiMainDlg.btnDoubleCount->SetText(strDoubleCount);
	m_DuiMainDlg.m_btnAccountAddr->SetText(DS_TRAY_WEB_STR);

	/*
	Json::Value data;  // data
	data["userid"] = Json::Value(m_LoginDlg.m_szUserID);

	Json::Value root;  // ��ʾ���� json ����
	root["data"] = data;
	root["userid"] = Json::Value(m_LoginDlg.m_szUserID);

	Json::FastWriter fast_writer;
	CString url = fast_writer.write(root).c_str();

	bool bret = true;
	if (g_pHttpJsonReader)
	{
		Json::Value value;
		Json::Value valArray;

		char strParam[1024] = {0};
		sprintf(strParam, "data=%s", url.GetBuffer(0));

		bool bSucceed = g_pHttpJsonReader->PostReadJson(m_LoginDlg.m_szServerAddr, "/clientapi/get_client_account", value, strParam);
		if (bSucceed)
		{
			CString tip = "";
			CString earnings = "";
			CString ordersNum = "";
			CString colorPrint1 = "";
			CString colorPrint0 = "";
			CString doublePrint1 = "";
			CString doublePrint0 = "";
			CString consumeSheet = "";
			CString clientAddr = "";

			char *ptip = NULL;
			CHttpJsonReader::UTF8Str2AnsiStr(value["tip"].asCString(), &ptip);
			tip.Format("%s", ptip);
			delete ptip;

			char *pearnings = NULL;
			CHttpJsonReader::UTF8Str2AnsiStr(value["earnings"].asCString(), &ptip);
			earnings.Format("%s", pearnings);
			delete pearnings;

			char *pordersNum = NULL;
			CHttpJsonReader::UTF8Str2AnsiStr(value["ordersNum"].asCString(), &ptip);
			ordersNum.Format("%s", pordersNum);
			delete pordersNum;

			char *pcolorPrint1 = NULL;
			CHttpJsonReader::UTF8Str2AnsiStr(value["colorPrint1"].asCString(), &ptip);
			colorPrint1.Format("%s", pcolorPrint1);
			delete pcolorPrint1;

			char *pcolorPrint0 = NULL;
			CHttpJsonReader::UTF8Str2AnsiStr(value["colorPrint0"].asCString(), &ptip);
			colorPrint0.Format("%s", pcolorPrint0);
			delete pcolorPrint0;

			char *pdoublePrint1 = NULL;
			CHttpJsonReader::UTF8Str2AnsiStr(value["doublePrint1"].asCString(), &ptip);
			doublePrint1.Format("%s", pdoublePrint1);
			delete pdoublePrint1;

			char *pdoublePrint0 = NULL;
			CHttpJsonReader::UTF8Str2AnsiStr(value["doublePrint0"].asCString(), &ptip);
			doublePrint0.Format("%s", pdoublePrint0);
			delete pdoublePrint0;

			char *pconsumeSheet = NULL;
			CHttpJsonReader::UTF8Str2AnsiStr(value["consumeSheet"].asCString(), &ptip);
			consumeSheet.Format("%s", pconsumeSheet);
			delete pconsumeSheet;

			char *pclientAddr = NULL;
			CHttpJsonReader::UTF8Str2AnsiStr(value["clientAddr"].asCString(), &ptip);
			clientAddr.Format("%s", pclientAddr);
			delete pclientAddr;
		}
		else
		{
			ShowTrayTips("������ʾ", "��ý�����������ʧ�ܣ�");
		}
	}
	else
	{
		ShowTrayTips("������ʾ", "��ý���������������������ϣ�����������");
	}
	*/
}

void CIPv6CloudDlg::SetSpeOrder(int nFlag)
{
	m_nSelMainNormal = nFlag;
	RefreshUnOrderList();
}

//�б�������˵�
void CIPv6CloudDlg::NormalMenu(int nFlag)
{
	m_nMainMenuFlag = nFlag;
	
	CPoint pos;
	GetCursorPos(&pos);
	Sleep(50);
	m_NormalMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, AfxGetMainWnd());
}

void CIPv6CloudDlg::DoubleMenu(int nFlag)
{
	m_nMainMenuFlag = nFlag;
	
	CPoint pos;
	GetCursorPos(&pos);
	Sleep(50);
	m_DoubleMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, AfxGetMainWnd());
}

void CIPv6CloudDlg::BlackMenu(int nFlag)
{
	m_nMainMenuFlag = nFlag;
	
	CPoint pos;
	GetCursorPos(&pos);
	Sleep(50);
	m_BlackMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, AfxGetMainWnd());
}

void CIPv6CloudDlg::OnBlackAll()
{
	if(m_nMainMenuFlag == 0)
	{
		m_nSelMainBlack = -1;
		m_DuiMainDlg.m_headMainBlack->SetText("��ɫ��");
		RefreshUnOrderList();
	}
	else
	{
		m_nSelHisBlack = -1;
		m_DuiMainDlg.m_headHisBlack->SetText("��ɫ��");
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::OnBlackBlack()
{
	if(m_nMainMenuFlag == 0)
	{
		m_nSelMainBlack = 0;
		m_DuiMainDlg.m_headMainBlack->SetText("�ڰר�");
		RefreshUnOrderList();
	}
	else
	{
		m_nSelHisBlack = 0;
		m_DuiMainDlg.m_headHisBlack->SetText("�ڰר�");
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::OnBlackColor()
{
	if(m_nMainMenuFlag == 0)
	{
		m_nSelMainBlack = 1;
		m_DuiMainDlg.m_headMainBlack->SetText("��ɫ��");
		RefreshUnOrderList();
	}
	else
	{
		m_nSelHisBlack = 1;
		m_DuiMainDlg.m_headHisBlack->SetText("��ɫ��");
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::OnDoubleAll()
{
	if(m_nMainMenuFlag == 0)
	{
		m_nSelMainDouble = -1;
		m_DuiMainDlg.m_headMainDouble->SetText("��˫��");
		RefreshUnOrderList();
	}
	else
	{
		m_nSelHisDouble = -1;
		m_DuiMainDlg.m_headHisDouble->SetText("��˫��");
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::OnDoubleSingle()
{
	if(m_nMainMenuFlag == 0)
	{
		m_nSelMainDouble = 0;
		m_DuiMainDlg.m_headMainDouble->SetText("���樋");
		RefreshUnOrderList();
	}
	else
	{
		m_nSelHisDouble = 0;
		m_DuiMainDlg.m_headHisDouble->SetText("���樋");
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::OnDoubleDouble()
{
	if(m_nMainMenuFlag == 0)
	{
		m_nSelMainDouble = 1;
		m_DuiMainDlg.m_headMainDouble->SetText("˫�樋");
		RefreshUnOrderList();
	}
	else
	{
		m_nSelHisDouble = 1;
		m_DuiMainDlg.m_headHisDouble->SetText("˫��");
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::OnNormalAll()
{
	if(m_nMainMenuFlag == 0)
	{
		m_nSelMainNormal = -1;
		m_DuiMainDlg.m_headMainNormal->SetText("���ͨ�");
		RefreshUnOrderList();
	}
	else
	{
		m_nSelHisNormal = -1;
		m_DuiMainDlg.m_headHisNormal->SetText("���ͨ�");
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::OnNormalNormal()
{
	if(m_nMainMenuFlag == 0)
	{
		m_nSelMainNormal = 0;
		m_DuiMainDlg.m_headMainNormal->SetText("��ͨ��");
		RefreshUnOrderList();
	}
	else
	{
		m_nSelHisNormal = 0;
		m_DuiMainDlg.m_headHisNormal->SetText("��ͨ��");
		RefreshHisOrderList();
	}
}

void CIPv6CloudDlg::OnNormalSpe()
{
	if(m_nMainMenuFlag == 0)
	{
		m_nSelMainNormal = 1;
		m_DuiMainDlg.m_headMainNormal->SetText("���⨋");
		RefreshUnOrderList();
	}
	else
	{
		m_nSelHisNormal = 1;
		m_DuiMainDlg.m_headHisNormal->SetText("���⨋");
		RefreshHisOrderList();
	}
}

//���¿ͻ�������״̬
void CIPv6CloudDlg::SetLoginStatus(int nStatus)
{
	Json::Value data;  // data
	data["online"] = Json::Value(nStatus);

	Json::Value root;  // ��ʾ���� json ����
	root["data"] = data;
	root["userid"] = Json::Value(m_LoginDlg.m_szUserID);

	Json::FastWriter fast_writer;
	CString url = fast_writer.write(root).c_str();

	bool bret = true;
	if (g_pHttpJsonReader)
	{
		Json::Value value;
		Json::Value valArray;

		char strParam[1024] = {0};
		sprintf(strParam, "data=%s", url.GetBuffer(0));

		bool bSucceed = g_pHttpJsonReader->PostReadJson(m_LoginDlg.m_szServerAddr, "/clientapi/update_online", value, strParam);
		if (bSucceed)
		{
			bool bRet = value["success"].asBool();

			if(bRet)
			{
			}
			else
			{
				char strErr[1024] = {0};
				char *pReason = NULL;
				CHttpJsonReader::UTF8Str2AnsiStr(value["reason"].asCString(), &pReason);
				strcpy(strErr, pReason);
				ShowTrayTips("������ʾ", pReason);
				delete pReason;
			}
		}
		else
		{
			ShowTrayTips("������ʾ", "���¿ͻ�������״̬ʧ�ܣ�");
		}
	}
	else
	{
		ShowTrayTips("������ʾ", "���¿ͻ�������״̬����������ϣ�����������");
	}
}

void CIPv6CloudDlg::Restart()
{
	//��������
	char pBuf[MAX_PATH];
	GetModuleFileName(NULL,pBuf,MAX_PATH);

	STARTUPINFO startupinfo;
	PROCESS_INFORMATION proc_info;
	memset(&startupinfo,0,sizeof(STARTUPINFO));
	startupinfo.cb=sizeof(STARTUPINFO);
	
	::CreateProcess(pBuf,NULL,NULL,NULL,FALSE, NORMAL_PRIORITY_CLASS,NULL,NULL,&startupinfo,&proc_info);
	SendMessage(WM_CLOSE, 0, 0);
}

void CIPv6CloudDlg::SetAutoLoginStatus(int nStatus)
{
	m_cGlobal.SaveAutoLoginStatus(CGlobal::g_ExePath, nStatus);
}