
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CIPv6CloudDlg �Ի���
CIPv6CloudDlg::CIPv6CloudDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIPv6CloudDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nHanding = 0;
	m_nTotalUnOrder = 0;
	m_nSpecialUnOrder = 0;
	m_nNodeliveryUnOrder = 0;
	m_nCurrentPage = 0;
	m_nHisCurrentPage = 0;
	m_nHisTotalOrder = 0;
	m_bWork = true;

	m_nSelMainNormal = -1;
	m_nSelMainDouble = -1;
	m_nSelMainBlack = -1;
	m_nSelHisNormal = -1;
	m_nSelHisDouble = -1;
	m_nSelHisBlack = -1;
	m_bAutoPrint = false;
}

CIPv6CloudDlg::~CIPv6CloudDlg()
{
	m_bWork = false;
}

void CIPv6CloudDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIPv6CloudDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_COMMAND(DS_TRAYMENU_SHOW, &CIPv6CloudDlg::OnTrayMenuShow)
	ON_COMMAND(DS_TRAYMENU_WEB, &CIPv6CloudDlg::OnTrayMenuWeb)
	ON_COMMAND(DS_TRAYMENU_MEMFREE, &CIPv6CloudDlg::OnTrayMenuMemfree)
	ON_COMMAND(DS_TRAYMENU_SYSSET, &CIPv6CloudDlg::OnTrayMenuSysset)
	ON_COMMAND(DS_TRAYMENU_ABOUT, &CIPv6CloudDlg::OnTrayMenuAbout)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_COMMAND(ID_MAIN_DOWNS, &CIPv6CloudDlg::OnMainDowns)
	ON_COMMAND(ID_MAIN_PRINTS, &CIPv6CloudDlg::OnMainPrints)
	ON_COMMAND(ID_MAIN_SETSTATUS_1, &CIPv6CloudDlg::OnMainSetstatus1)
	ON_COMMAND(ID_MAIN_SETSTATUS_2, &CIPv6CloudDlg::OnMainSetstatus2)
	ON_COMMAND(ID_MIAN_R_DOWN, &CIPv6CloudDlg::OnMianRDown)
	ON_COMMAND(ID_MIAN_R_PRINT, &CIPv6CloudDlg::OnMianRPrint)
	ON_COMMAND(ID_MAIN_R_SETSTATUS_1, &CIPv6CloudDlg::OnMainRSetstatus1)
	ON_COMMAND(ID_MAIN_R_SETSTATUS_2, &CIPv6CloudDlg::OnMainRSetstatus2)
	ON_COMMAND(ID_PRINT_R_ADDTO_B, &CIPv6CloudDlg::OnPrintRAddtoB)
	ON_COMMAND(ID_PRINT_R_ADDTO_C, &CIPv6CloudDlg::OnPrintRAddtoC)
	ON_COMMAND(ID_PRINT_R_DELETE, &CIPv6CloudDlg::OnPrintRDelete)
	ON_COMMAND(ID_BLACK_ALL, &CIPv6CloudDlg::OnBlackAll)
	ON_COMMAND(ID_BLACK_BLACK, &CIPv6CloudDlg::OnBlackBlack)
	ON_COMMAND(ID_BLACK_COLOR, &CIPv6CloudDlg::OnBlackColor)
	ON_COMMAND(ID_DOUBLE_ALL, &CIPv6CloudDlg::OnDoubleAll)
	ON_COMMAND(ID_DOUBLE_SINGLE, &CIPv6CloudDlg::OnDoubleSingle)
	ON_COMMAND(ID_DOUBLE_DOUBLE, &CIPv6CloudDlg::OnDoubleDouble)
	ON_COMMAND(ID_NORMAL_ALL, &CIPv6CloudDlg::OnNormalAll)
	ON_COMMAND(ID_NORMAL_NORMAL, &CIPv6CloudDlg::OnNormalNormal)
	ON_COMMAND(ID_NORMAL_SPE, &CIPv6CloudDlg::OnNormalSpe)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CIPv6CloudDlg)
	//EASYSIZE(IDOK,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
END_EASYSIZE_MAP

void CIPv6CloudDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		if(nID == SC_RESTORE)
		{
		}
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CIPv6CloudDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CIPv6CloudDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CIPv6CloudDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	UPDATE_EASYSIZE;
}

void CIPv6CloudDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnSizing(fwSide, pRect);
    EASYSIZE_MINSIZE(500,400,fwSide,pRect);
}

BOOL CIPv6CloudDlg::PreTranslateMessage(MSG* pMsg) 
{
	/* �������϶����� */ 
	if ((pMsg->message == WM_LBUTTONDOWN))
	{
		/* ���Static Region�������� */
		CRect rectTopRegion;
		GetWindowRect(&rectTopRegion);

		CRect rectLeft = rectTopRegion;
		rectLeft.right = rectLeft.left + 168;

		CRect rectTop = rectTopRegion;
		rectTop.bottom = rectTop.top + 50;
		rectTop.left = rectTop.left + 620;
		rectTop.right = rectTop.right - 150;

		/* �����굱ǰ����λ�� */
		POINT pt;
		GetCursorPos(&pt);

		/* �ж�����Ƿ���ϵͳ��ť���� */
		/* (PtInRect:��������ж�ָ���ĵ��Ƿ�λ�ھ���lpRect�ڲ�) */
		if(rectTop.PtInRect(pt) || rectLeft.PtInRect(pt))
		{
			//����������ϵͳ��ť�����ڣ���������϶�������Ϣ
			CPoint point;
			//SetCursor(LoadCursor(NULL, IDC_SIZEALL));
			PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
			return TRUE;
		}
	}
	else if(pMsg->message == WM_LBUTTONDBLCLK)
	{
		/* ���Static Region�������� */
		CRect rectTopRegion;
		GetWindowRect(&rectTopRegion);

		CRect rectLeft = rectTopRegion;
		rectLeft.right = rectLeft.left + 168;

		CRect rectTop = rectTopRegion;
		rectTop.bottom = rectTop.top + 50;
		rectTop.left = rectTop.left + 620;
		rectTop.right = rectTop.right - 150;

		/* �����굱ǰ����λ�� */
		POINT pt;
		GetCursorPos(&pt);

		/* �ж�����Ƿ���ϵͳ��ť���� */
		/* (PtInRect:��������ж�ָ���ĵ��Ƿ�λ�ھ���lpRect�ڲ�) */
		if(rectTop.PtInRect(pt) || rectLeft.PtInRect(pt))
		{
			if (m_DuiMainDlg.IsMax())
			{
				RestoreWndByDui();
				m_DuiMainDlg.SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
			}
			else
			{
				m_DuiMainDlg.SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				MaxWndByDui();
			}
			return TRUE;
		}
	}
	else if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_RETURN)
		{
			m_DuiMainDlg.Search();
		}
		else if(pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
	}
	else
	{
		//TRACE("\n=========PreTranslateMessage========%d, %d\n", pMsg->message, pMsg->wParam);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CIPv6CloudDlg::MinWndByDui()
{
	ShowWindow(SW_MINIMIZE);
}

void CIPv6CloudDlg::MaxWndByDui()
{
	//ShowWindow(SW_MAXIMIZE);
	int cx = GetSystemMetrics(SM_CXFULLSCREEN);
	int cy = GetSystemMetrics(SM_CYFULLSCREEN);
	CRect rt;
	SystemParametersInfo(SPI_GETWORKAREA,0,&rt,0);
	cy = rt.bottom;
	MoveWindow(0, 0, cx, cy);

	HRGN hRgn;  
    RECT rect;
	GetWindowRect(&rect);  
    hRgn = CreateRoundRectRgn(0, 0, rect.right - rect.left, rect.bottom - rect.top, 5, 5);  
    SetWindowRgn(hRgn, TRUE);

	m_WndShadow.Show(m_hWnd);
}

void CIPv6CloudDlg::RestoreWndByDui()
{
	//ShowWindow(SW_RESTORE);
	MoveWindow(0, 0, 890, 590);

	HRGN hRgn;  
    RECT rect;
	GetWindowRect(&rect);  
    hRgn = CreateRoundRectRgn(0, 0, rect.right - rect.left, rect.bottom - rect.top, 5, 5);  
    SetWindowRgn(hRgn, TRUE);

	CenterWindow();
	m_WndShadow.Show(m_hWnd);
}

int CIPv6CloudDlg::GetIPv6()
{
	int nType = 0;
	CCollector co;
	CString strContent = co.Open("http://ip6.me/", FALSE);
	if(strContent.Find(DS_ERROR_FLAG) < 0)
	{
		CString strValue = "";
		int nPos = co.GetKey(strContent, "<font color=\"#FF0000\">", "</font>", 0, strValue);
		if(strValue == "IPv4")
		{
			nType = 1;
		}
		else
		{
			nType = 2;
		}

		nPos = co.GetKey(strContent, "<font face=\"Arial, Monospace\" size=+3>", "</font>", nPos, strValue);
		if(strValue != "")
		{
			strcpy(m_szClientAddr, strValue.GetBuffer(0));
		}
		else
		{
			nType = -1;
		}
	}
	else
	{
		nType = -1;
	}
	return nType;
}
//IPv4