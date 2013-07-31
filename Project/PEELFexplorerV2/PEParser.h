// PEParser.h: interface for the CPEParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PEPARSER_H__98C6E043_4366_40E1_A406_BAABE11AD556__INCLUDED_)
#define AFX_PEPARSER_H__98C6E043_4366_40E1_A406_BAABE11AD556__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CoffParser.h"

typedef enum
{
    PE_TABLE_UNKOWN = 0,
    PE_DOS_HEADER = 1000,
    PE_COFF_HEADER = 2000,
    PE_OPTIONAL_HEADER = 3000,
    PE_SECTION_HEADER = 4000,
    PE_DIRECTORY_EXPORT = 5000,
    PE_DIRECTORY_IMPORT = 6000,
    PE_DIRECTORY_RESOURCE = 7000,
    PE_DIRECTORY_BASERELOC = 8000,
    PE_DIRECTORY_DEBUG = 9000,
    PE_DIRECTORY_TLS = 10000
}PE_TABLE_TYPE;

class CPEParser : public CCoffParser  
{
public:
	CPEParser();
	virtual ~CPEParser();

    virtual BOOL OpenFile(char *pName);
    virtual BOOL CloseFile();
    virtual BOOL ShowFileTree(CTreeCtrl *pFileTree);
    virtual BOOL UpdateLstCtrl(CReportCtrl *pfileRepList, NM_TREEVIEW* pNMTreeView);
    
public:
    BYTE *m_pPEData;
    IMAGE_DOS_HEADER *m_pImgDosHdr;
    IMAGE_NT_HEADERS32 *m_pImgNTHdr;
    IMAGE_FILE_HEADER *m_pImgFileHdr;
    IMAGE_OPTIONAL_HEADER32 *m_pImgOptHdr;
    IMAGE_SECTION_HEADER *m_pImgSctHdr;
};

#endif // !defined(AFX_PEPARSER_H__98C6E043_4366_40E1_A406_BAABE11AD556__INCLUDED_)
