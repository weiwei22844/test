#pragma once


// CMyRichEdit

class CMyRichEdit : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CMyRichEdit)

public:
	CMyRichEdit();
	virtual ~CMyRichEdit();    

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnCopy() { Copy(); }
    afx_msg void OnSelectall() { SetSel(0, -1); }
    afx_msg void OnClearAll();
};


