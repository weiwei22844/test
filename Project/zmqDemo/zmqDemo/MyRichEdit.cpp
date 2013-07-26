// MyRichEdit.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MyRichEdit.h"

#define ID_RICH_UNDO                    101
#define ID_RICH_CUT                     102
#define ID_RICH_COPY                    103
#define ID_RICH_PASTE                   104
#define ID_RICH_CLEAR                   105
#define ID_RICH_SELECTALL               106
#define ID_RICH_SETFONT                 107

// CMyRichEdit

IMPLEMENT_DYNAMIC(CMyRichEdit, CRichEditCtrl)

CMyRichEdit::CMyRichEdit()
{
   
}

CMyRichEdit::~CMyRichEdit()
{
}


BEGIN_MESSAGE_MAP(CMyRichEdit, CRichEditCtrl)
    ON_WM_RBUTTONDOWN()
    ON_COMMAND(ID_RICH_COPY, OnCopy)
    ON_COMMAND(ID_RICH_SELECTALL, OnSelectall)
    ON_COMMAND(ID_RICH_CLEAR, OnClearAll)
END_MESSAGE_MAP()



// CMyRichEdit ��Ϣ�������



void CMyRichEdit::OnRButtonDown(UINT nFlags, CPoint point)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    LONG lCharHeight;
    CHARFORMAT cf;
    memset(&cf, 0, sizeof(CHARFORMAT));
    GetDefaultCharFormat(cf);
    
    //int nline = GetLineCount();

    int StartIndex = LineIndex(0);  
    int EndIndex = LineIndex(1);  
    if (EndIndex != -1)  
    {
        CPoint LinePoint = GetCharPos(StartIndex);
        CPoint AfterPoint = GetCharPos(EndIndex);
        lCharHeight = AfterPoint.y - LinePoint.y;
    }else
    {
        lCharHeight = cf.yHeight/15;
    }

    //����Ϊ����
    SetFocus();
    //����һ������ʽ�˵�
    CMenu popmenu;
    popmenu.CreatePopupMenu();
    //��Ӳ˵���Ŀ
    //popmenu.AppendMenu(0, ID_RICH_UNDO, "&Undo");
    //popmenu.AppendMenu(0, MF_SEPARATOR);
    //popmenu.AppendMenu(0, ID_RICH_CUT, "&Cut");
    popmenu.AppendMenu(0, ID_RICH_COPY, _T("����"));
    //popmenu.AppendMenu(0, ID_RICH_PASTE, "&Paste");
    popmenu.AppendMenu(0, ID_RICH_SELECTALL, _T("ѡ��ȫ������"));
    popmenu.AppendMenu(0, MF_SEPARATOR);
    popmenu.AppendMenu(0, ID_RICH_CLEAR, _T("���ȫ������"));
    //popmenu.AppendMenu(0, MF_SEPARATOR);
    //popmenu.AppendMenu(0, ID_RICH_SETFONT, "Select &Font");

    //��ʼ���˵���
    //UINT nUndo=(CanUndo() ? 0 : MF_GRAYED );
    //popmenu.EnableMenuItem(ID_RICH_UNDO, MF_BYCOMMAND|nUndo);

    CPoint CurrPoint;
    CurrPoint = GetCaretPos();
    long nStartChar;
    long nEndChar;
    GetSel(nStartChar, nEndChar);
    CPoint startPoint;
    CPoint endPoint;
    startPoint = GetCharPos(nStartChar);            // Gets the position (top-left corner) of a given character within this CRichEditCtrl
    endPoint = GetCharPos(nEndChar);

    if(point.y < startPoint.y || point.y > endPoint.y + lCharHeight 
        || (point.x < startPoint.x && point.y > startPoint.y && point.y < startPoint.y + lCharHeight)
        || (point.x > endPoint.x && point.y > endPoint.y && point.y < endPoint.y + lCharHeight))
    {
        //SetSel(0, -1);
        SetSel(-1, 0);                          // ȡ���ı�ѡ��
    }

    CurrPoint.y = point.y/lCharHeight*lCharHeight;
    CurrPoint.x = point.x;
    SetCaretPos(CurrPoint);
    //char chTT[100];
    //sprintf_s(chTT, "y: %d startY: %d endY: %d", CurrPoint.y, startPoint.y, endPoint.y);
    //OutputDebugStringA(chTT);

    UINT nSel=((GetSelectionType()!=SEL_EMPTY) ? 0 : MF_GRAYED) ;
    //popmenu.EnableMenuItem(ID_RICH_CUT, MF_BYCOMMAND|nSel);
    popmenu.EnableMenuItem(ID_RICH_COPY, MF_BYCOMMAND|nSel);
    //popmenu.EnableMenuItem(ID_RICH_CLEAR, MF_BYCOMMAND|nSel);

    //UINT nPaste=(CanPaste() ? 0 : MF_GRAYED) ;
    //popmenu.EnableMenuItem(ID_RICH_PASTE, MF_BYCOMMAND|nPaste);

    //��ʾ�˵�
    CPoint pt;
    GetCursorPos(&pt);
    popmenu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
    popmenu.DestroyMenu();

    CRichEditCtrl::OnRButtonDown(nFlags, point);
}

void CMyRichEdit::OnClearAll()
{
    SetWindowText(_T(""));
}
