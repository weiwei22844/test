// PEParser.cpp: implementation of the CPEParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PEELFexplorer.h"
#include "PEParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*#define IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor*/
char sz_desc_dirEntry_type[][128] = { "Export Directory", "Import Directory", "Resource Directory", "Exception Directory", 
                                    "Security Directory", "Base Relocation Table", "Debug Directory", "Architecture Specific Data", 
                                    "RVA of GP", "TLS Directory", "Load Configuration Directory", "Bound Import Directory in headers",  
                                    "Import Address Table", "Delay Load Import Descriptors", "COM Runtime descriptor", "Undefined"};
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPEParser::CPEParser()
{
    m_pPEData = NULL;
    m_pImgDosHdr = NULL;
    m_pImgNTHdr = NULL;
    m_pImgFileHdr = NULL;
    m_pImgOptHdr = NULL;
    m_pImgSctHdr = NULL;
    m_fileType = COFF_TYPE_PE;
}

CPEParser::~CPEParser()
{
    
}

BOOL CPEParser::OpenFile(char *pName)
{
    BOOL bRt = FALSE;
    if(!pName)
    {
        TRACE("CPEParser can not open file NULL\n");
        return bRt;
    }
    
    if(m_pPEData)
    {
        CloseFile();            // 关闭原有打开文件
    }
    
    HANDLE hFile = CreateFile(pName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == NULL)
    {
        TRACE("CPEParser can not open file: %s\n", pName);
        return bRt;
    }	
    
    //得到文件长度
    DWORD dwDatalen;
    dwDatalen = GetFileSize(hFile, NULL);
    if (dwDatalen <= sizeof(IMAGE_DOS_HEADER))
    {
        CloseHandle(hFile);
        TRACE("File %s length image dos header error in CPEParser::OpenFile\n", pName);
        return bRt;
    }
    
    m_pPEData = (BYTE*)malloc(dwDatalen);
    if(!m_pPEData)
    {
        CloseHandle(hFile);
        TRACE("Malloc memory for File %s failed in CPEParser::OpenFile\n", pName);
        return bRt;
    }
    
    LARGE_INTEGER li;
    li.QuadPart = 0;
    li.LowPart = SetFilePointer(hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
    DWORD dwReadLen = 0;
    bRt = ReadFile(hFile, m_pPEData, dwDatalen, &dwReadLen, NULL);
    if(bRt && (dwDatalen == dwReadLen))
    {
        m_pImgDosHdr = (IMAGE_DOS_HEADER* )m_pPEData;
        if(sizeof(IMAGE_DOS_HEADER) + m_pImgDosHdr->e_lfanew >= dwReadLen)
        {
            CloseHandle(hFile);
            free(m_pPEData);
            m_pPEData = NULL;
            TRACE("File %s length PE header error in CPEParser::OpenFile\n", pName);
            return bRt;
        }
        m_pImgNTHdr = (IMAGE_NT_HEADERS32*)(m_pPEData + m_pImgDosHdr->e_lfanew);
        if(m_pImgNTHdr->Signature != 0x4550)
        {
            CloseHandle(hFile);
            free(m_pPEData);
            m_pPEData = NULL;
            TRACE("PE signature error of Image nt header in CPEParser::OpenFile\n", pName);
            return bRt;
        }
        m_pImgFileHdr = &(m_pImgNTHdr->FileHeader);
        m_pImgOptHdr =  &(m_pImgNTHdr->OptionalHeader);
        if(dwReadLen >= m_pImgDosHdr->e_lfanew + sizeof(IMAGE_NT_HEADERS32))
            m_pImgSctHdr = (IMAGE_SECTION_HEADER*)(m_pPEData + m_pImgDosHdr->e_lfanew + sizeof(IMAGE_NT_HEADERS32));
    }
    
    return TRUE;
}

BOOL CPEParser::CloseFile()
{
    if(m_pPEData)
    {
        free(m_pPEData);
        m_pPEData = NULL;
        m_pImgDosHdr = NULL;
        m_pImgNTHdr = NULL;
        m_pImgFileHdr = NULL;
        m_pImgOptHdr = NULL;
        m_pImgSctHdr = NULL;
        m_fileType = COFF_TYPE_PE;
    }

    return TRUE;
}

BOOL CPEParser::ShowFileTree(CTreeCtrl *pFileTree)
{
    TV_INSERTSTRUCT TreeCtrlItem;
    char chTemp[100];
    
    if(!m_pImgDosHdr)
    {
        return FALSE;
    }
    
    // ELF 文件头
    TreeCtrlItem.hParent = TVI_ROOT;
    TreeCtrlItem.hInsertAfter = TVI_LAST;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
    TreeCtrlItem.item.pszText = _T("Dos Header");
    TreeCtrlItem.item.lParam = PE_DOS_HEADER;
    HTREEITEM hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);

    TreeCtrlItem.hParent = TVI_ROOT;
    TreeCtrlItem.hInsertAfter = TVI_LAST;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
    TreeCtrlItem.item.pszText = _T("Coff Header");
    TreeCtrlItem.item.lParam = PE_COFF_HEADER;
    hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);

    TreeCtrlItem.hParent = TVI_ROOT;
    TreeCtrlItem.hInsertAfter = TVI_LAST;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
    TreeCtrlItem.item.pszText = _T("Optional Header");
    TreeCtrlItem.item.lParam = PE_OPTIONAL_HEADER;
    hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);


    TreeCtrlItem.hParent = TVI_ROOT;
    TreeCtrlItem.hInsertAfter = TVI_LAST;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
    TreeCtrlItem.item.pszText = _T("Section Header");
    TreeCtrlItem.item.lParam = PE_SECTION_HEADER;
    hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);
    if(m_pImgSctHdr)
    {
        for(int i = 1; i <= m_pImgFileHdr->NumberOfSections; i++)
        {
            TreeCtrlItem.hParent = hTreeItem0;
            sprintf(chTemp, "%s", m_pImgSctHdr[i-1].Name);
            TreeCtrlItem.item.pszText = chTemp;
            TreeCtrlItem.item.lParam = PE_SECTION_HEADER+i;
            pFileTree->InsertItem(&TreeCtrlItem);
        }
    }
    
    // 导出表
    if(m_pImgOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size > 0)
    {
        TreeCtrlItem.hParent = TVI_ROOT;
        TreeCtrlItem.hInsertAfter = TVI_LAST;
        TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
        TreeCtrlItem.item.pszText = _T("导出表");
        TreeCtrlItem.item.lParam = PE_DIRECTORY_EXPORT;
        hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);
    }

    // 导入表
    if(m_pImgOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size > 0)
    {
        TreeCtrlItem.hParent = TVI_ROOT;
        TreeCtrlItem.hInsertAfter = TVI_LAST;
        TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
        TreeCtrlItem.item.pszText = _T("导入表");
        TreeCtrlItem.item.lParam = PE_DIRECTORY_IMPORT;
        hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);
    }

    // 资源表
    if(m_pImgOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size > 0)
    {
        TreeCtrlItem.hParent = TVI_ROOT;
        TreeCtrlItem.hInsertAfter = TVI_LAST;
        TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
        TreeCtrlItem.item.pszText = _T("资源表");
        TreeCtrlItem.item.lParam = PE_DIRECTORY_RESOURCE;
        hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);
    }

    // 重定位表
    if(m_pImgOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size > 0)
    {
        TreeCtrlItem.hParent = TVI_ROOT;
        TreeCtrlItem.hInsertAfter = TVI_LAST;
        TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
        TreeCtrlItem.item.pszText = _T("重定位表");
        TreeCtrlItem.item.lParam = PE_DIRECTORY_BASERELOC;
        hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);
    }

    // 调试信息表
    if(m_pImgOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size > 0)
    {
        TreeCtrlItem.hParent = TVI_ROOT;
        TreeCtrlItem.hInsertAfter = TVI_LAST;
        TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
        TreeCtrlItem.item.pszText = _T("调试信息表");
        TreeCtrlItem.item.lParam = PE_DIRECTORY_DEBUG;
        hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);
    }

    // TLS表
    if(m_pImgOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size > 0)
    {
        TreeCtrlItem.hParent = TVI_ROOT;
        TreeCtrlItem.hInsertAfter = TVI_LAST;
        TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
        TreeCtrlItem.item.pszText = _T("TLS表");
        TreeCtrlItem.item.lParam = PE_DIRECTORY_TLS;
        hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);
    }

    return TRUE;
}

BOOL CPEParser::UpdateLstCtrl(CReportCtrl *pfileRepList, NM_TREEVIEW* pNMTreeView)
{
    int nRow;
    DWORD dwAddr = 0;
    char chText[500];
    char chTmp[10];
    int i, nRowCnt;

    if(!m_pImgDosHdr)
    {
        return FALSE;
    }

    pfileRepList->DeleteAllItems();
    pfileRepList->DeleteAllColumns();
    switch(pNMTreeView->itemNew.lParam)
    {
    case PE_DOS_HEADER:
        // 插入相关列
        pfileRepList->InsertColumn(0, "地址", LVCFMT_LEFT, 60);
        pfileRepList->InsertColumn(1, "大小", LVCFMT_LEFT, 40);
        pfileRepList->InsertColumn(2, "十六进制取值", LVCFMT_LEFT, 120);
        pfileRepList->InsertColumn(3, "含义", LVCFMT_LEFT, 200);

        // 依次设置行内容
        dwAddr = 0;
        nRowCnt = 0;
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);                 // 插入行
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_magic));               // 设置大小列内容
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_magic);
        pfileRepList->SetItemText(nRow, 2, chText);                         // 设置取值列内容 魔数
        sprintf(chText, "%s", "Magic为MZ(0x4D 0x5A)");
        pfileRepList->SetItemText(nRow, 3, chText);                         // 设置含义列内容

        dwAddr += sizeof(m_pImgDosHdr->e_magic);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_cblp));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_cblp);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "bytes on last page of file");

        dwAddr += sizeof(m_pImgDosHdr->e_cblp);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_cp));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_cp);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Pages in file");

        dwAddr += sizeof(m_pImgDosHdr->e_cp);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_crlc));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_crlc);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Relocatons");

        dwAddr += sizeof(m_pImgDosHdr->e_crlc);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_cparhdr));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_cparhdr);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Size of header in paragraphs");
        
        dwAddr += sizeof(m_pImgDosHdr->e_cparhdr);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_minalloc));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_minalloc);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Minimum extra paragraphs needed");

        dwAddr += sizeof(m_pImgDosHdr->e_minalloc);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_maxalloc));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_maxalloc);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Maximum extra paragraphs needed");

        dwAddr += sizeof(m_pImgDosHdr->e_maxalloc);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_ss));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_ss);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Initial(relative) SS value");

        dwAddr += sizeof(m_pImgDosHdr->e_ss);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_sp));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_sp);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Initial SP value");
        
        dwAddr += sizeof(m_pImgDosHdr->e_sp);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_csum));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_csum);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Checksum");
        
        dwAddr += sizeof(m_pImgDosHdr->e_csum);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_ip));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_ip);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Initial IP value");
        
        dwAddr += sizeof(m_pImgDosHdr->e_ip);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_cs));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_cs);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Initial CS value");
        
        dwAddr += sizeof(m_pImgDosHdr->e_cs);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_lfarlc));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_lfarlc);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "File address of relocation table");
        
        dwAddr += sizeof(m_pImgDosHdr->e_lfarlc);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_ovno));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_ovno);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Overlay number");
        
        dwAddr += sizeof(m_pImgDosHdr->e_ovno);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_res));
        pfileRepList->SetItemText(nRow, 1, chText);
        memset(chText, 0, sizeof(chText));
        for(i = 0; i < sizeof(m_pImgDosHdr->e_res)/sizeof(m_pImgDosHdr->e_res[0]); i++)
        {
            sprintf(chTmp, "%02x", m_pImgDosHdr->e_res[i]);
            strcat(chText, chTmp);
        }
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Reserved words");

        dwAddr += sizeof(m_pImgDosHdr->e_res);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_oemid));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_oemid);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "OEM identifier(for e_oeminfo)");
        
        dwAddr += sizeof(m_pImgDosHdr->e_oemid);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_oeminfo));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_oeminfo);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "OEM information(e_oemid specific)");
        
        dwAddr += sizeof(m_pImgDosHdr->e_oeminfo);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_res2));
        pfileRepList->SetItemText(nRow, 1, chText);
        memset(chText, 0, sizeof(chText));
        for(i = 0; i < sizeof(m_pImgDosHdr->e_res2)/sizeof(m_pImgDosHdr->e_res2[0]); i++)
        {
            sprintf(chTmp, "%02x", m_pImgDosHdr->e_res2[i]);
            strcat(chText, chTmp);
        }
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Reserved words");
        
        dwAddr += sizeof(m_pImgDosHdr->e_res2);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgDosHdr->e_lfanew));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgDosHdr->e_lfanew);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "File address of new exe header");
		break;
    case PE_COFF_HEADER:
        // 插入相关列
        pfileRepList->InsertColumn(0, "地址", LVCFMT_LEFT, 60);
        pfileRepList->InsertColumn(1, "大小", LVCFMT_LEFT, 40);
        pfileRepList->InsertColumn(2, "十六进制取值", LVCFMT_LEFT, 120);
        pfileRepList->InsertColumn(3, "含义", LVCFMT_LEFT, 200);
        
        // 依次设置行内容
        dwAddr = m_pImgDosHdr->e_lfanew;
        nRowCnt = 0;
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgNTHdr->Signature));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgNTHdr->Signature);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Signature");

        dwAddr += sizeof(m_pImgNTHdr->Signature);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgFileHdr->Machine));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgFileHdr->Machine);
        pfileRepList->SetItemText(nRow, 2, chText);
        sprintf(chText, "%s", "Machine");
        pfileRepList->SetItemText(nRow, 3, chText);
        
        dwAddr += sizeof(m_pImgFileHdr->Machine);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgFileHdr->NumberOfSections));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgFileHdr->NumberOfSections);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Number of sections");

        dwAddr += sizeof(m_pImgFileHdr->NumberOfSections);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgFileHdr->TimeDateStamp));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pImgFileHdr->TimeDateStamp);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Time date stamp");

        dwAddr += sizeof(m_pImgFileHdr->TimeDateStamp);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgFileHdr->PointerToSymbolTable));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pImgFileHdr->PointerToSymbolTable);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Pointer to symbol table");
        
        dwAddr += sizeof(m_pImgFileHdr->PointerToSymbolTable);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgFileHdr->NumberOfSymbols));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pImgFileHdr->NumberOfSymbols);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Number of symbols");

        dwAddr += sizeof(m_pImgFileHdr->NumberOfSymbols);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgFileHdr->SizeOfOptionalHeader));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgFileHdr->SizeOfOptionalHeader);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Size of optional header");
        
        dwAddr += sizeof(m_pImgFileHdr->SizeOfOptionalHeader);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgFileHdr->Characteristics));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgFileHdr->Characteristics);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Characteristics");
        break;
    case PE_OPTIONAL_HEADER:
        pfileRepList->InsertColumn(0, "地址", LVCFMT_LEFT, 60);
        pfileRepList->InsertColumn(1, "大小", LVCFMT_LEFT, 40);
        pfileRepList->InsertColumn(2, "十六进制取值", LVCFMT_LEFT, 120);
        pfileRepList->InsertColumn(3, "含义", LVCFMT_LEFT, 200);

        dwAddr = m_pImgDosHdr->e_lfanew+sizeof(m_pImgNTHdr->Signature)+sizeof(m_pImgNTHdr->FileHeader);
        nRowCnt = 0;
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgOptHdr->Magic));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgOptHdr->Magic);
        pfileRepList->SetItemText(nRow, 2, chText);
        sprintf(chText, "%s", "Magic");
        pfileRepList->SetItemText(nRow, 3, chText);

        dwAddr += sizeof(m_pImgOptHdr->Magic);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgOptHdr->MajorLinkerVersion));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%02x", m_pImgOptHdr->MajorLinkerVersion);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Major linker version");
        
        dwAddr += sizeof(m_pImgOptHdr->MajorLinkerVersion);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgOptHdr->MinorLinkerVersion));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%02x", m_pImgOptHdr->MinorLinkerVersion);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Minor linker version");

        dwAddr += sizeof(m_pImgOptHdr->MinorLinkerVersion);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgOptHdr->SizeOfCode));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pImgOptHdr->SizeOfCode);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Size of code");

        dwAddr += sizeof(m_pImgOptHdr->SizeOfCode);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgOptHdr->SizeOfInitializedData));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pImgOptHdr->SizeOfInitializedData);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Size of initialized data");

        dwAddr += sizeof(m_pImgOptHdr->SizeOfInitializedData);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgOptHdr->SizeOfUninitializedData));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pImgOptHdr->SizeOfUninitializedData);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Size of uninitialized data");

        dwAddr += sizeof(m_pImgOptHdr->SizeOfUninitializedData);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgOptHdr->AddressOfEntryPoint));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pImgOptHdr->AddressOfEntryPoint);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Address of entry point");
        
        dwAddr += sizeof(m_pImgOptHdr->AddressOfEntryPoint);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgOptHdr->BaseOfCode));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pImgOptHdr->BaseOfCode);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Base of code");
        
        dwAddr += sizeof(m_pImgOptHdr->BaseOfCode);
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgOptHdr->BaseOfData));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pImgOptHdr->BaseOfData);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "Base of data");

        if(m_pImgFileHdr->SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER32))
        {
            dwAddr += sizeof(m_pImgOptHdr->BaseOfData);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->ImageBase));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->ImageBase);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Image base");
            
            dwAddr += sizeof(m_pImgOptHdr->ImageBase);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->SectionAlignment));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->SectionAlignment);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Section alignment");
            
            dwAddr += sizeof(m_pImgOptHdr->SectionAlignment);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->FileAlignment));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->FileAlignment);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "File alignment");

            dwAddr += sizeof(m_pImgOptHdr->FileAlignment);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->MajorOperatingSystemVersion));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%04x", m_pImgOptHdr->MajorOperatingSystemVersion);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Major operating system version");

            dwAddr += sizeof(m_pImgOptHdr->MajorOperatingSystemVersion);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->MinorOperatingSystemVersion));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%04x", m_pImgOptHdr->MinorOperatingSystemVersion);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Minor operating system version");

            dwAddr += sizeof(m_pImgOptHdr->MinorOperatingSystemVersion);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->MajorImageVersion));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%04x", m_pImgOptHdr->MajorImageVersion);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Major iamge version");

            dwAddr += sizeof(m_pImgOptHdr->MajorImageVersion);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->MinorImageVersion));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%04x", m_pImgOptHdr->MinorImageVersion);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Minor image version");

            dwAddr += sizeof(m_pImgOptHdr->MinorImageVersion);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->MajorSubsystemVersion));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%04x", m_pImgOptHdr->MajorSubsystemVersion);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Major subsystem version");

            dwAddr += sizeof(m_pImgOptHdr->MajorSubsystemVersion);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->MinorSubsystemVersion));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%04x", m_pImgOptHdr->MinorSubsystemVersion);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Minor subsystem version");

            dwAddr += sizeof(m_pImgOptHdr->MinorSubsystemVersion);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->Win32VersionValue));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->Win32VersionValue);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Win32 version value");

            dwAddr += sizeof(m_pImgOptHdr->Win32VersionValue);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->SizeOfImage));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->SizeOfImage);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Size of image");

            dwAddr += sizeof(m_pImgOptHdr->SizeOfImage);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->SizeOfHeaders));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->SizeOfHeaders);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Size of headers");

            dwAddr += sizeof(m_pImgOptHdr->SizeOfHeaders);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->CheckSum));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->CheckSum);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Check sum");

            dwAddr += sizeof(m_pImgOptHdr->CheckSum);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->Subsystem));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%04x", m_pImgOptHdr->Subsystem);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Subsystem");
            
            dwAddr += sizeof(m_pImgOptHdr->Subsystem);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->DllCharacteristics));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%04x", m_pImgOptHdr->DllCharacteristics);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Dll Characteristics");

            dwAddr += sizeof(m_pImgOptHdr->DllCharacteristics);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->SizeOfStackReserve));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->SizeOfStackReserve);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Size of stack reserve");
            
            dwAddr += sizeof(m_pImgOptHdr->SizeOfStackReserve);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->SizeOfStackCommit));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->SizeOfStackCommit);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Size of stack commit");
            
            dwAddr += sizeof(m_pImgOptHdr->SizeOfStackCommit);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->SizeOfHeapReserve));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->SizeOfHeapReserve);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Size of heap reserve");
            
            dwAddr += sizeof(m_pImgOptHdr->SizeOfHeapReserve);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->SizeOfHeapCommit));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->SizeOfHeapCommit);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Size of heap commit");

            dwAddr += sizeof(m_pImgOptHdr->SizeOfHeapCommit);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->LoaderFlags));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->LoaderFlags);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Loader flags");
            
            dwAddr += sizeof(m_pImgOptHdr->LoaderFlags);
            sprintf(chText, "%08x", dwAddr);
            nRow = pfileRepList->InsertItem(nRowCnt++, chText);
            sprintf(chText, "%d", sizeof(m_pImgOptHdr->NumberOfRvaAndSizes));
            pfileRepList->SetItemText(nRow, 1, chText);
            sprintf(chText, "%08x", m_pImgOptHdr->NumberOfRvaAndSizes);
            pfileRepList->SetItemText(nRow, 2, chText);
            pfileRepList->SetItemText(nRow, 3, "Number of RvaAndSizes");

            dwAddr += sizeof(m_pImgOptHdr->NumberOfRvaAndSizes);
            for(i = 0; i < m_pImgOptHdr->NumberOfRvaAndSizes; i++)
            {
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgOptHdr->DataDirectory[i].VirtualAddress));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgOptHdr->DataDirectory[i].VirtualAddress);
                pfileRepList->SetItemText(nRow, 2, chText);
                sprintf(chText, "Address of %s", sz_desc_dirEntry_type[i]);
                pfileRepList->SetItemText(nRow, 3, chText);
                dwAddr += sizeof(m_pImgOptHdr->DataDirectory[i].VirtualAddress);

                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgOptHdr->DataDirectory[i].Size));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgOptHdr->DataDirectory[i].Size);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "Size");
                dwAddr += sizeof(m_pImgOptHdr->DataDirectory[i].Size);
            }
        }
        break;
    case PE_SECTION_HEADER:
        break;
    case PE_DIRECTORY_EXPORT:
        /*pfileRepList->InsertColumn(0, "地址", LVCFMT_LEFT, 60);
        pfileRepList->InsertColumn(1, "大小", LVCFMT_LEFT, 40);
        pfileRepList->InsertColumn(2, "十六进制取值", LVCFMT_LEFT, 120);
        pfileRepList->InsertColumn(3, "含义", LVCFMT_LEFT, 200);
        
        nRowCnt = 0;
        sprintf(chText, "%08x", m_pImgOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
        sprintf(chText, "%d", sizeof(m_pImgOptHdr->Magic));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pImgOptHdr->Magic);
        pfileRepList->SetItemText(nRow, 2, chText);
        sprintf(chText, "%s", "Magic");
        pfileRepList->SetItemText(nRow, 3, chText);*/
        break;
    case PE_DIRECTORY_IMPORT:
        {
            pfileRepList->InsertColumn(0, "地址", LVCFMT_LEFT, 60);
            pfileRepList->InsertColumn(1, "大小", LVCFMT_LEFT, 40);
            pfileRepList->InsertColumn(2, "十六进制取值", LVCFMT_LEFT, 120);
            pfileRepList->InsertColumn(3, "含义", LVCFMT_LEFT, 200);
            
            nRowCnt = 0;
            dwAddr = m_pImgOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
            IMAGE_IMPORT_DESCRIPTOR *pImgImport = (IMAGE_IMPORT_DESCRIPTOR *)(m_pPEData+m_pImgOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
            for(i = 0; i < m_pImgOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size/sizeof(IMAGE_IMPORT_DESCRIPTOR); i++)
            {
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(pImgImport->OriginalFirstThunk));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", pImgImport->OriginalFirstThunk);
                pfileRepList->SetItemText(nRow, 2, chText);
                sprintf(chText, "%s", "Magic");
                pfileRepList->SetItemText(nRow, 3, chText);

                dwAddr += sizeof(pImgImport->OriginalFirstThunk);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(pImgImport->TimeDateStamp));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", pImgImport->TimeDateStamp);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "当可执行文件不与被输入的DLL进行绑定时，此字段为0");

                dwAddr += sizeof(pImgImport->TimeDateStamp);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(pImgImport->ForwarderChain));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", pImgImport->ForwarderChain);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "第一个被转向的API");
                
                pImgImport += sizeof(IMAGE_IMPORT_DESCRIPTOR);
            }
        }
        break;
    case PE_DIRECTORY_RESOURCE:
        break;
    case PE_DIRECTORY_BASERELOC:
        break;
    case PE_DIRECTORY_DEBUG:
        break;
    case PE_DIRECTORY_TLS:
        break;
    default:
        if(pNMTreeView->itemNew.lParam > PE_SECTION_HEADER && pNMTreeView->itemNew.lParam < PE_DIRECTORY_EXPORT)
        {
            pfileRepList->InsertColumn(0, "地址", LVCFMT_LEFT, 60);
            pfileRepList->InsertColumn(1, "大小", LVCFMT_LEFT, 40);
            pfileRepList->InsertColumn(2, "十六进制取值", LVCFMT_LEFT, 120);
            pfileRepList->InsertColumn(3, "含义", LVCFMT_LEFT, 200);

            int nIndex = pNMTreeView->itemNew.lParam - PE_SECTION_HEADER - 1;
            if(nIndex < m_pImgFileHdr->NumberOfSections)
            {
                dwAddr = m_pImgDosHdr->e_lfanew+sizeof(IMAGE_NT_HEADERS32)+nIndex*sizeof(IMAGE_SECTION_HEADER);
                nRowCnt = 0;
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgSctHdr[nIndex].Name));
                pfileRepList->SetItemText(nRow, 1, chText);
                memset(chText, 0, sizeof(chText));
                for(i = 0; i < sizeof(m_pImgSctHdr[nIndex].Name); i++)
                {
                    sprintf(chTmp, "%02x", m_pImgSctHdr[nIndex].Name[i]);
                    strcat(chText, chTmp);
                }
                pfileRepList->SetItemText(nRow, 2, chText);
                sprintf(chText, "Section name: %s", m_pImgSctHdr[nIndex].Name);
                pfileRepList->SetItemText(nRow, 3, chText);

                dwAddr += sizeof(m_pImgSctHdr[nIndex].Name);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgSctHdr[nIndex].Misc.VirtualSize));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgSctHdr[nIndex].Misc.VirtualSize);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "被使用的区块大小");

                dwAddr += sizeof(m_pImgSctHdr[nIndex].Misc.VirtualSize);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgSctHdr[nIndex].VirtualAddress));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgSctHdr[nIndex].VirtualAddress);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "该块装载到内存中的RVA");

                dwAddr += sizeof(m_pImgSctHdr[nIndex].VirtualAddress);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgSctHdr[nIndex].SizeOfRawData));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgSctHdr[nIndex].SizeOfRawData);
                pfileRepList->SetItemText(nRow, 2, chText);
                sprintf(chText, "%s", "该块在磁盘文件中所占的大小");
                pfileRepList->SetItemText(nRow, 3, chText);

                dwAddr += sizeof(m_pImgSctHdr[nIndex].SizeOfRawData);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgSctHdr[nIndex].PointerToRawData));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgSctHdr[nIndex].PointerToRawData);
                pfileRepList->SetItemText(nRow, 2, chText);
                sprintf(chText, "%s", "该块在磁盘文件中的偏移");
                pfileRepList->SetItemText(nRow, 3, chText);

                dwAddr += sizeof(m_pImgSctHdr[nIndex].PointerToRawData);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgSctHdr[nIndex].PointerToRelocations));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgSctHdr[nIndex].PointerToRelocations);
                pfileRepList->SetItemText(nRow, 2, chText);
                sprintf(chText, "%s", "在exe文件中无意义");
                pfileRepList->SetItemText(nRow, 3, chText);

                dwAddr += sizeof(m_pImgSctHdr[nIndex].PointerToRelocations);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgSctHdr[nIndex].PointerToLinenumbers));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgSctHdr[nIndex].PointerToLinenumbers);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "指向节中行号项开头的文件指针");

                dwAddr += sizeof(m_pImgSctHdr[nIndex].PointerToLinenumbers);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgSctHdr[nIndex].NumberOfRelocations));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgSctHdr[nIndex].NumberOfRelocations);
                pfileRepList->SetItemText(nRow, 2, chText);
                sprintf(chText, "%s", "节中重定位项的个数");
                pfileRepList->SetItemText(nRow, 3, chText);
        
                dwAddr += sizeof(m_pImgSctHdr[nIndex].NumberOfRelocations);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgSctHdr[nIndex].NumberOfLinenumbers));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgSctHdr[nIndex].NumberOfLinenumbers);
                pfileRepList->SetItemText(nRow, 2, chText);
                sprintf(chText, "%s", "节中行号项的个数");
                pfileRepList->SetItemText(nRow, 3, chText);
        
                dwAddr += sizeof(m_pImgSctHdr[nIndex].NumberOfLinenumbers);
                sprintf(chText, "%08x", dwAddr);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                sprintf(chText, "%d", sizeof(m_pImgSctHdr[nIndex].Characteristics));
                pfileRepList->SetItemText(nRow, 1, chText);
                sprintf(chText, "%08x", m_pImgSctHdr[nIndex].Characteristics);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "描述节特征的标志");
            }
        }
        break;
    }

    return TRUE;
}
