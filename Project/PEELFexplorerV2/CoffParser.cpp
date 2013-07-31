// CoffParser.cpp: implementation of the CCoffParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PEELFexplorer.h"
#include "CoffParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define READ_LEN 10
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCoffParser::CCoffParser()
{
    m_fileType = FILE_TYPE_UNKOWN;
}

CCoffParser::~CCoffParser()
{

}

BOOL CCoffParser::OpenFile(char *pName)
{
    BOOL bRt = FALSE;
    if(!pName)
    {
        TRACE("CCoffParser can not open file NULL\n");
        return bRt;
    }
    HANDLE hFile = CreateFile(pName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == NULL)
    {
        TRACE("CCoffParser can not open file: %s\n", pName);
        return bRt;
    }	
    
    //得到文件长度
    DWORD dwDatalen;
    dwDatalen = GetFileSize(hFile, NULL);
    if (dwDatalen <= READ_LEN)
    {
        CloseHandle(hFile);
        TRACE("File %s length error in CCoffParser::OpenFile\n", pName);
        return bRt;
	}
    m_fileType = FILE_TYPE_UNKOWN;

    LARGE_INTEGER li;
    li.QuadPart = 0;
    li.LowPart = SetFilePointer(hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
    char chFileHeader[READ_LEN];
    DWORD dwReadLen = 0;
	bRt = ReadFile(hFile, &chFileHeader, READ_LEN, &dwReadLen, NULL);
    if(bRt && dwReadLen == READ_LEN)
    {
        char chMagicELF[5] = { 0x7f, 0x45, 0x4c, 0x46};
        char chMagicPE[3] = { 0x4D, 0x5A};
        if(memcmp(chFileHeader, chMagicELF, strlen(chMagicELF)) == 0)
        {
            m_fileType = COFF_TYPE_ELF;
        }else if(memcmp(chFileHeader, chMagicPE, strlen(chMagicPE)) == 0)
        {
             m_fileType = COFF_TYPE_PE;
        }
    }
    
    return TRUE;
}

BOOL CCoffParser::CloseFile()
{
    return FALSE;
}

COFF_FILE_TYPE CCoffParser::GetFileType()
{
    return m_fileType;
}

