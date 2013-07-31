// CoffParser.h: interface for the CCoffParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COFFPARSER_H__529D7CFD_6530_49D7_BE20_1D60FD0B4C12__INCLUDED_)
#define AFX_COFFPARSER_H__529D7CFD_6530_49D7_BE20_1D60FD0B4C12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ReportCtrl.h"

typedef enum
{
    FILE_TYPE_UNKOWN = 0,
    COFF_TYPE_ELF,
    COFF_TYPE_PE
}COFF_FILE_TYPE;

class CCoffParser  
{
public:
	CCoffParser();
	virtual ~CCoffParser();

    virtual BOOL OpenFile(char *pName);
    virtual BOOL CloseFile();
    virtual BOOL ShowFileTree(CTreeCtrl *pFileTree){ return FALSE; }
    virtual BOOL UpdateLstCtrl(CReportCtrl *pfileRepList, NM_TREEVIEW* pNMTreeView) { return FALSE; }
    COFF_FILE_TYPE GetFileType();
public:
    COFF_FILE_TYPE m_fileType;
};

#endif // !defined(AFX_COFFPARSER_H__529D7CFD_6530_49D7_BE20_1D60FD0B4C12__INCLUDED_)
