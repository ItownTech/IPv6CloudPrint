
// IPv6CloudDlg.h : ͷ�ļ�
//
#include "Comm\DS_Lock.h"
#include "Comm\TCPSocket.h"
#include "Comm\DS_Print.h"
#include "Comm\FileDownload.h"
#include "Comm\DS_Utils.h"
#include "Comm\Collector.h"

#include "DuiDlg\DuiLoginDlg.h"
#include "DuiDlg\DuiMainDlg.h"
#include "DuiDlg\WndShadow.h"

#include "LoginDlg.h"

#include <io.h>

#pragma once

// CIPv6CloudDlg �Ի���
class CIPv6CloudDlg : public CDialogEx
{
	DECLARE_EASYSIZE
public:
	// ʵ��
	CTCPSocket m_TCP;
	// δ��ɶ�����ʾ�б�
	vector<ST_ORDER> m_UnOrderList;
	DSLock m_UnOrderListLock;
	int    m_nTotalUnOrder; //�ܶ�����
	int    m_nSpecialUnOrder; //���ⶩ����
	int    m_nNodeliveryUnOrder; //δ���Ͷ�����
	int    m_nCurrentPage; //��ǰҳ��
	// ��ʷ������ʾ�б�
	vector<ST_ORDER> m_HisOrderList;
	DSLock m_HisOrderListLock;
	int    m_nHisTotalOrder; //�ܶ�����
	int    m_nHisCurrentPage; //��ǰҳ��
	// Print
	vector<ST_ORDER> m_PrintOrderList;
	DSLock m_PrintOrderListLock;
	

	// ����IPv6��ַ
	char m_szClientAddr[256];

	//ý���ļ����ط���
	CFileDownload m_FileDownload;

	bool  m_bWork; //���߳��Ƿ���
	int   m_nHanding; //���ڴ���Ķ�����
	// ȫ�������ļ���Ϣ
	CGlobal m_cGlobal;

	// �����ֻ���
	CString  m_strMainSearch;
	CString  m_strHisSearch;
	// �Ҽ��˵���־
	int  m_nMainMenuFlag;
	int  m_nPrintMenuFlag;
	//����
	void Search(int nIndex, CString strSearch);
	int  m_nSelMainNormal;
	int  m_nSelMainDouble;
	int  m_nSelMainBlack;
	int  m_nSelHisNormal;
	int  m_nSelHisDouble;
	int  m_nSelHisBlack;

	//
	bool m_bAutoPrint;
public:
	// ���½���ʾ
	void ShowTrayTips(CString strTitle, CString strContent);
	// �ڴ�����
	void ToMemFree();
	// ��С��
	void MinWndByDui();
	// ���
	void MaxWndByDui();
	// �ָ�
	void RestoreWndByDui();
	//��ӡ
	bool Print(CString strFileName, ST_ORDER stOrder);
	void PrintHis();
	// ��ѡ�񱣴�·��
	void OpenPath();
	void PathBrowser();
	// ���δ��ɶ���
	int GetUnOrderList(int nPage);
	void RefreshUnOrderList();
	// ���δ��ɶ���
	int GetHisOrderList(int nPage);
	void RefreshHisOrderList();
	void GetAndRefresh(int nFlag);
	// ����ļ����ص�ַ
	CString GetDownloadFileUrl(ST_ORDER stOrder);
	// ��ÿͻ��˵�ַ
	void GetClientAddr();
	// �����������
	int SaveCfg(CString strLocalIpv6, CString strServerIpv6, CString strSavePath, CString strColorPrint, CString strBlackPrint);
	// �����б�
	CString ExportHis(CString strName = "", int nFlag = 1);
	// �����ҳ���ð�ť
	void OnSetStatusBtn();
	// ���ÿͻ��˴�����״̬
	bool SetOrderStatus(char *szOrderId, int nPayStatus, int nPrintStatus, int nSendStatus);
	// ���ö���״̬
	void UpdateUnOrderListStatus(FileItem * pMFI, bool bStatus, bool bHanding);
	// ��ҳ�Ҽ��б�
	void  MainRightDown(int nFlag);
	// ��ӡ���Ҽ��б�
	void  PrintRDown(int nFlag);
	// ��õ�ǰ��ѡ����б��� �������
	void GetListOrderST(ST_ORDER &stOrder);
	CString GetListOrderVec();
	void PrePage(int nFlag);
	void NextPage(int nFlag);
	void GotoPage(int nFlag, int nPage);
	void Refresh(int nFlag);
	void SelAll(int nFlag);

	void RDown(ST_ORDER stOrder, bool bPath);
	void RPrint(ST_ORDER stOrder);
	void RSetstatus1(ST_ORDER stOrder);
	void RSetstatus2(ST_ORDER stOrder);

	// ���ÿͻ��˵�ַ
	void SetClientAddr(CString strIPAddr);

	// �������
	void GetAccount();

	//�Ƿ�ֻ��ʾ���ⶩ��
	void SetSpeOrder(int nFlg);

	//�б�������˵�
	void NormalMenu(int nFlag);
	void DoubleMenu(int nFlag);
	void BlackMenu(int nFlag);

	//���¿ͻ�������״̬
	void SetLoginStatus(int nStatus);

	int GetIPv6();
	
	void Restart();

	void SetAutoLoginStatus(int nStatus);
private:
	// ���δ��ɶ����߳�
	static void PrintHisSvc(void * pParam);
	// ��ö����߳�
	static void GetHisOrderListSvc(void * pParam);
	// �Զ���ӡ�߳�
	static void AutoPrintSvc(void * pParam);

	// �ڴ�����ʱ����
	int m_nFreeMemTime;

	// �ڴ������߳�
	static void FreeMemSvc(void * pParam);

	//���ݻص�
	static void DataArrivedCB(char *data, int length, DWORD userdata);

	//״̬�ص�
	static void StatusChangeCB(char *data, int length, DWORD userdata);

	//Dui�Ի���
	CLoginDlg m_LoginDlg;
	CDuiMainDlg m_DuiMainDlg;
	CWndShadow m_WndShadow;

	//��ӡ���б�
	CStringArray m_arrPrintList;
protected:
	HICON m_hIcon;

	/*  -----------------------------------------���½����̼��˵����------------------------------------------->*/
	// ��ʼ������
	int InitTray();

	// �������̲˵�CMenu����
	CMenu m_TrayMenu;
	
	// ������ҳCMenu����
	CMenu m_MainMenu;
	CMenu m_MainListMenu;
	CMenu m_PrintRMenu;

	// ������ҳCMenu����
	CMenu m_NormalMenu;
	CMenu m_DoubleMenu;
	CMenu m_BlackMenu;

	// �˵����ͼ��
	CBitmap m_TrayMainBmp;
	CBitmap m_TraySyssetBmp;
	CBitmap m_TrayExitBmp;
	CBitmap m_TrayAboutBmp;
	CBitmap m_TrayWebBmp;
	CBitmap m_TrayClearBmp;

	/* ����ͼ������ */
	NOTIFYICONDATA m_NotifyIconData;

	/*<------------------------------------------���½����̼��˵����----------------------------------------------*/

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	// ����
public:
	CIPv6CloudDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CIPv6CloudDlg();	// ��׼���캯��

	// �Ի�������
	enum { IDD = IDD_IPv6Cloud_DIALOG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	afx_msg void OnDestroy();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg void OnTrayMenuShow();
	afx_msg void OnTrayMenuWeb();
	afx_msg void OnTrayMenuMemfree();
	afx_msg void OnTrayMenuSysset();
	afx_msg void OnTrayMenuAbout();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnMainDowns();
	afx_msg void OnMainPrints();
	afx_msg void OnMainSetstatus1();
	afx_msg void OnMainSetstatus2();
	afx_msg void OnMianRDown();
	afx_msg void OnMianRPrint();
	afx_msg void OnMainRSetstatus1();
	afx_msg void OnMainRSetstatus2();
	afx_msg void OnPrintRAddtoB();
	afx_msg void OnPrintRAddtoC();
	afx_msg void OnPrintRDelete();
	afx_msg void OnBlackAll();
	afx_msg void OnBlackBlack();
	afx_msg void OnBlackColor();
	afx_msg void OnDoubleAll();
	afx_msg void OnDoubleSingle();
	afx_msg void OnDoubleDouble();
	afx_msg void OnNormalAll();
	afx_msg void OnNormalNormal();
	afx_msg void OnNormalSpe();
};

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
public:
};