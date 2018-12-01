/*===================================================*\
|                                                     |
|  EASY-SIZE Macros                                   |
|                                                     |
|  Copyright (c) 2001 - Marc Richarme                 |
|      devix@devix.cjb.net                            |
|      http://devix.cjb.net                           |
|                                                     |
|  License:                                           |
|                                                     |
|  You may use this code in any commersial or non-    |
|  commersial application, and you may redistribute   |
|  this file (and even modify it if you wish) as      |
|  long as you keep this notice untouched in any      |
|  version you redistribute.                          |
|                                                     |
|  Usage:                                             |
|                                                     |
|  - Insert 'DECLARE_EASYSIZE' somewhere in your      |
|    class declaration                                |
|  - Insert an easysize map in the beginning of your  |
|    class implementation (see documentation) and     |
|    outside of any function.                         |
|  - Insert 'INIT_EASYSIZE;' in your                  |
|    OnInitDialog handler.                            |
|  - Insert 'UPDATE_EASYSIZE' in your OnSize handler  |
|  - Optional: Insert 'EASYSIZE_MINSIZE(mx,my);' in   |
|    your OnSizing handler if you want to specify     |
|    a minimum size for your dialog                   |
|                                                     |
|        Check http://devix.cjb.net for the           |
|              docs and new versions                  |
|                                                     |
\*===================================================*/


#ifndef __EASYSIZE_H_
#define __EASYSIZE_H_
#define ES_BORDER 0xffffffff
#define ES_KEEPSIZE 0xfffffffe
#define ES_HCENTER 0x00000001
#define ES_VCENTER 0x00000002
#define DECLARE_EASYSIZE \
void __ES__RepositionControls(BOOL bInit);\
void __ES__CalcBottomRight(CWnd *pThis, BOOL bBottom, int &bottomright, int &topleft, UINT id, UINT br, int es_br, CRect &rect, int clientbottomright);
#define INIT_EASYSIZE __ES__RepositionControls(TRUE); __ES__RepositionControls(FALSE)
#define UPDATE_EASYSIZE if(GetWindow(GW_CHILD)!=NULL) __ES__RepositionControls(FALSE)
#define EASYSIZE_MINSIZE(mx,my,s,r) if(r->right-r->left < mx) { if((s == WMSZ_BOTTOMLEFT)||(s == WMSZ_LEFT)||(s == WMSZ_TOPLEFT)) r->left = r->right-mx; else r->right = r->left+mx; } if(r->bottom-r->top < my) { if((s == WMSZ_TOP)||(s == WMSZ_TOPLEFT)||(s == WMSZ_TOPRIGHT)) r->top = r->bottom-my; else r->bottom = r->top+my; }
#define BEGIN_EASYSIZE_MAP(class) \
void class::__ES__CalcBottomRight(CWnd *pThis, BOOL bBottom, int &bottomright, int &topleft, UINT id, UINT br, int es_br, CRect &rect, int clientbottomright) {\
if(br==ES_BORDER) bottomright = clientbottomright-es_br;\
else if(br==ES_KEEPSIZE) bottomright = topleft+es_br;\
else { CRect rect2;\
pThis->GetDlgItem(br)->GetWindowRect(rect2); pThis->ScreenToClient(rect2);\
bottomright = (bBottom?rect2.top:rect2.left) - es_br;}}\
void class::__ES__RepositionControls(BOOL bInit) { CRect rect,rect2,client; GetClientRect(client);
#define END_EASYSIZE_MAP Invalidate(); UpdateWindow(); }
#define EASYSIZE(id,l,t,r,b,o) \
static int id##_es_l, id##_es_t, id##_es_r, id##_es_b;\
if(bInit) {\
GetDlgItem(id)->GetWindowRect(rect); ScreenToClient(rect);\
if(o & ES_HCENTER) id##_es_l = rect.Width()/2; else {\
if(l==ES_BORDER) id##_es_l = rect.left; else if(l==ES_KEEPSIZE) id##_es_l = rect.Width(); else {\
	GetDlgItem(l)->GetWindowRect(rect2); ScreenToClient(rect2);\
	id##_es_l = rect.left-rect2.right;}}\
if(o & ES_VCENTER) id##_es_t = rect.Height()/2; else {\
if(t==ES_BORDER) id##_es_t = rect.top; else if(t==ES_KEEPSIZE) id##_es_t = rect.Height(); else {\
	GetDlgItem(t)->GetWindowRect(rect2); ScreenToClient(rect2);\
	id##_es_t = rect.top-rect2.bottom;}}\
if(o & ES_HCENTER) id##_es_r = rect.Width(); else { if(r==ES_BORDER) id##_es_r = client.right-rect.right; else if(r==ES_KEEPSIZE) id##_es_r = rect.Width(); else {\
	GetDlgItem(r)->GetWindowRect(rect2); ScreenToClient(rect2);\
	id##_es_r = rect2.left-rect.right;}}\
if(o & ES_VCENTER) id##_es_b = rect.Height(); else  { if(b==ES_BORDER) id##_es_b = client.bottom-rect.bottom; else if(b==ES_KEEPSIZE) id##_es_b = rect.Height(); else {\
	GetDlgItem(b)->GetWindowRect(rect2); ScreenToClient(rect2);\
	id##_es_b = rect2.top-rect.bottom;}}\
} else {\
int left,top,right,bottom; BOOL bR = FALSE,bB = FALSE;\
if(o & ES_HCENTER) { int _a,_b;\
if(l==ES_BORDER) _a = client.left; else { GetDlgItem(l)->GetWindowRect(rect2); ScreenToClient(rect2); _a = rect2.right; }\
if(r==ES_BORDER) _b = client.right; else { GetDlgItem(r)->GetWindowRect(rect2); ScreenToClient(rect2); _b = rect2.left; }\
left = _a+((_b-_a)/2-id##_es_l); right = left + id##_es_r;} else {\
if(l==ES_BORDER) left = id##_es_l;\
else if(l==ES_KEEPSIZE) { __ES__CalcBottomRight(this,FALSE,right,left,id,r,id##_es_r,rect,client.right); left = right-id##_es_l;\
} else { GetDlgItem(l)->GetWindowRect(rect2); ScreenToClient(rect2); left = rect2.right + id##_es_l; }\
if(l != ES_KEEPSIZE) __ES__CalcBottomRight(this,FALSE,right,left,id,r,id##_es_r,rect,client.right);}\
if(o & ES_VCENTER) { int _a,_b;\
if(t==ES_BORDER) _a = client.top; else { GetDlgItem(t)->GetWindowRect(rect2); ScreenToClient(rect2); _a = rect2.bottom; }\
if(b==ES_BORDER) _b = client.bottom; else { GetDlgItem(b)->GetWindowRect(rect2); ScreenToClient(rect2); _b = rect2.top; }\
top = _a+((_b-_a)/2-id##_es_t); bottom = top + id##_es_b;} else {\
if(t==ES_BORDER) top = id##_es_t;\
else if(t==ES_KEEPSIZE) { __ES__CalcBottomRight(this,TRUE,bottom,top,id,b,id##_es_b,rect,client.bottom); top = bottom-id##_es_t;\
} else { GetDlgItem(t)->GetWindowRect(rect2); ScreenToClient(rect2); top = rect2.bottom + id##_es_t; }\
if(t != ES_KEEPSIZE) __ES__CalcBottomRight(this,TRUE,bottom,top,id,b,id##_es_b,rect,client.bottom);}\
GetDlgItem(id)->MoveWindow(left,top,right-left,bottom-top,FALSE);\
}
#endif //__EASYSIZE_H_

/************************************************************************/
/* 
EASYSIZE(IDC_LOG_LIST,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,ES_HCENTER)

left, top,right,bottom�ĸ�����Ϊ�ؼ�λ�õ����꣬������ѡ��ES_BORDER, ES_KEEPSIZE,�ؼ�IDֵ����֮һ��

options����ΪES_HCENTER, ES_VCENTER�Ľ�ϣ�options����0��

ES_BORDER��ʾ�ؼ���Ի���߽磨���¼�Ʊ߽磩���룻

ES_KEEPSIZE��ʾ�ؼ�ˮƽ/��ֱ�����ϵĳߴ籣�ֲ��䣻

�ؼ�IDֵ��ʾ��ǰ�ؼ���ָ���ؼ�֮��ľ��룻

ES_HCENTER��ʾ���ź�ؼ���ָ��λ����ˮƽ���У�

ES_VCENTER��ʾ���ź�ؼ���ָ��λ���ڴ�ֱ���У�

////////////////////////////////////////////////////////////////////////////////////////////////////////
ʹ��˵��
����һ
a��#include EasySize.h������stdafx.h
b���߽�#include EasySize.h������TestCurveDlg.h�У���ֱ�ӷ�����Ҫʹ�õĴ����ļ�
ʵ��Ӧ��ʱ������b��ʽ���������ֱ������
�����
��DECLARE_EASYSIZE��ӵ��κ�һ���������У�TestCurveDlg.h��
class CTestCurveDlg : public CDialog
{
DECLARE_EASYSIZE
   ������
����OnInitDialog����������в����ڵĻ�����������ԭ�����У�����"INIT_EASYSIZE;"��ӽ�ȥ(TestCurveDlg.cpp)
BOOL CTestCurveDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
... ...   
        INIT_EASYSIZE;
        return TRUE; // return TRUE  unless you set the focus to a control
}
  ������
����OnSize����UPDATE_EASYSIZE��ӽ�ȥ(TestCurveDlg.cpp)
void CTestCurveDlg::OnSize(UINT nType, int cx, int cy)
{
         CDialog::OnSize(nType, cx, cy);
       UPDATE_EASYSIZE; //���´��ڴ�С
         �������
CRect rect;
         GetWindowRect(rect);
         ScreenToClient(rect);
         CRect ctrrect;
         CWnd* pWnd ;
 
    pWnd = GetDlgItem(IDC_STATIC_CURVE);  //���»�ͼ���С
         if(pWnd)
         {
                   if (m_pCurveCtrl)
                   {
                            m_pCurveCtrl->GetWindowRect(ctrrect);
                            ScreenToClient(ctrrect);
 
                            CRect newrect(ctrrect.left, ctrrect.top, rect.right-120, rect.bottom);
                            m_pCurveCtrl->MoveWindow(newrect);
                   }
         }
  ������
�����Ҫ�û����õ���С�����ܣ����Դ���OnSizing�����EASYSIZE_MINSIZE������ɲ���EasySize_demo��
void CEasySizeDemoDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
    CDialog::OnSizing(fwSide, pRect);
    EASYSIZE_MINSIZE(280,250,fwSide,pRect);
}
//(in this example, 280 is the minimum width and 250 the minimum height we want our dialog to have)
  �����������Ĳ��֣�
����һ��"EasySize Map"�����ܣ����÷Ŵ�/��С����ʱ����ʾЧ�������Է�������κ�λ�á�
BEGIN_EASYSIZE_MAP(class_name)
    ...
    EASYSIZE(control,left,top,right,bottom,options)
    ...
END_EASYSIZE_MAP
  �����������£�TestCurveDlg.cpp��
BEGIN_EASYSIZE_MAP(CTestCurveDlg)  //�Ŵ󴰿�
EASYSIZE(IDC_STATIC,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel0, ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel1, ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel2, ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel3, ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel4, ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel5, ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel6, ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel7, ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel8, ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel9, ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel10,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel11,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel12,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel13,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel14,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_CHECK_Channel15,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
EASYSIZE(IDC_BUTTON_OpenFile,ES_KEEPSIZE,ES_KEEPSIZE,ES_BORDER,ES_BORDER,0)
EASYSIZE(IDC_BUTTON_ZoomOut, ES_KEEPSIZE,ES_KEEPSIZE,ES_BORDER,ES_BORDER,0)
EASYSIZE(IDC_BUTTON_Resort,  ES_KEEPSIZE,ES_KEEPSIZE,ES_BORDER,ES_BORDER,0)
EASYSIZE(IDC_STATIC_CURVE,ES_BORDER,ES_BORDER,IDC_STATIC,IDC_BUTTON_ZoomOut,0)
END_EASYSIZE_MAP
...
ע��
EASYSIZE(control,left,top,right,bottom,options)
Control---�ؼ���ID����Ҫ���õĿؼ�
left top right bottom---���ο��λ�ã��������õĲ�����ES_BORDER��ES_KEEPSIZE������control��ID��
        ES_BORDER����ؼ���߿�ľ��뱣�ֲ���
        ES_KEEPSIZE����ؼ��Ŀ�Ⱥ͸߶ȱ��ֲ��䣬left/rightֻ����������һ��ΪES_KEEPSIZE,top/bottom��Ȼ��
Control  ID��������һ���ռ�Ϊ����
        ����ע�����£������ÿؼ���ΪID1�����տؼ���ΪID2
        i left=ID2  ID1��ߵ�ID2�ұߵľ��뱣�ֲ��䣨����ID2��α仯��
        ii right=ID2 ID1�ұߵ�ID2��ߵľ��뱣�ֲ��䣨����ID2��α仯��
iii top=ID2  ID1�ϱߵ�ID2�±ߵľ��뱣�ֲ��䣨����ID2��α仯��
iv bottom=ID2 ID1�±ߵ�ID2�ϱߵľ��뱣�ֲ��䣨����ID2��α仯��
options---�ر����ES_HCENTER, ES_VCENTER and 0�Ľ�ϡ�
0����û���κβ�����
ES_HCENTER����ؼ���left/right֮��ˮƽ���У���ʱleft/right���������ó�ES_KEEPSIZE���ؼ��Ŀ�ȱ��ֲ��䣻
ES_VCENTERͬ��ES_HCENTER
/************************************************************************/