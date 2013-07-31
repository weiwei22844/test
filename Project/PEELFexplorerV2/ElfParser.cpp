// ElfParser.cpp: implementation of the CElfParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PEELFexplorer.h"
#include "ElfParser.h"
#include "elfEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

char sz_desc_e_type[][128] = { "No file type", "Relocatable file", "Executable file", "Shared object file", "Core file" };
char sz_desc_e_machine[][30] = {"No machine", "AT&T", "SPARC", "Intel 80386", "Motorola m68k", "Motorola m88k", "undefined", "Intel 80860", "MIPS R3000 big-endian", "IBM System/370", "MIPS R3000 little-endian", \
    "undefined", "undefined", "undefined", "undefined", \
    "HPPA", "undefined", "Fujitsu VPP500", "v8plus", "Intel 80960", "PowerPC", "PowerPC 64-bit", "IBM S390", \
    "undefined", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined", "undefined", \
    "NEC V800 series", "Fujitsu FR20", "TRW RH-32", "Motorola RCE", "ARM", "Digital Alpha", "Hitachi SH", "SPARC v9 64-bit", "Siemens Tricore", "Argonaut RISC Core", "Hitachi H8/300" \
    "Hitachi H8/300H", "Hitachi H8S", "Hitachi H8/500", "Intel Merced", "Stanford MIPS-X", "Motorola Coldfire", "Motorola M68HC12", "Fujitsu MMA", "undefined", "Siemens PCP", "Sony nCPU embeeded RISC", \
    "Denso NDR1", "Motorola Start*Core", "Toyota ME16", "STMicroelectronic ST100", "Tinyj emb.fam", "AMD x86-64", "Sony DSP"\
};
char sz_desc_p_type[][128] = {"PT_NULL", "PT_LOAD", "PT_DYNAMIC", "PT_INTERP", "PT_NOTE", "PT_SHLIB", "PT_PHDR"};
char sz_desc_sh_type[][128] = {	"SHT_NULL", "SHT_PROGBITS", "SHT_SYMTAB", "SHT_STRTAB",
                                "SHT_RELA", "SHT_HASH", "SHT_DYNAMIC", "SHT_NOTE",
                                "SHT_NOBITS", "SHT_REL", "SHT_SHLIB", "SHT_DYNSYM"};	
char sz_desc_sh_flags[][128] = {"Unknown", "SHF_WRITE", "SHF_ALLOC", "SHF_WRITE & SHF_ALLOC", "SHF_EXECINSTR",
                                "SHF_WRITE & SHF_ALLOC", "SHF_ALLOC & SHF_EXECINSTR",
                                "SHF_WRITE & SHF_ALLOC & SHF_EXECINSTR"};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CElfParser::CElfParser()
{
    m_pFileData = NULL;
    m_pElf_Ehdr = NULL;
    m_pElf_Phdr = NULL;
    m_pElf_Shdr = NULL;
    m_pElf_SstrTbl = NULL;
    m_nSStrTblLen = 0;
    m_fileType = COFF_TYPE_ELF;
}

CElfParser::~CElfParser()
{
    if(m_pFileData)
    {
        free(m_pFileData);
    }
}

BOOL CElfParser::OpenFile(char *pName)
{
    BOOL bRt = FALSE;
    if(!pName)
    {
        TRACE("CElfParser can not open file NULL\n");
        return bRt;
    }

    if(m_pFileData)
    {
        CloseFile();            // 关闭原有打开文件
    }

    HANDLE hFile = CreateFile(pName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == NULL)
    {
        TRACE("CElfParser can not open file: %s\n", pName);
        return bRt;
    }	
    
    //得到文件长度
    DWORD dwDatalen;
    dwDatalen = GetFileSize(hFile, NULL);
    if (dwDatalen <= sizeof(Elf32_Ehdr))
    {
        CloseHandle(hFile);
        TRACE("File %s length error in CElfParser::OpenFile\n", pName);
        return bRt;
    }

    m_pFileData = (BYTE*)malloc(dwDatalen);
    if(!m_pFileData)
    {
        CloseHandle(hFile);
        TRACE("Malloc memory for File %s failed in CElfParser::OpenFile\n", pName);
        return bRt;
    }

    LARGE_INTEGER li;
    li.QuadPart = 0;
    li.LowPart = SetFilePointer(hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
    DWORD dwReadLen = 0;
    bRt = ReadFile(hFile, m_pFileData, dwDatalen, &dwReadLen, NULL);
    if(bRt && (dwDatalen == dwReadLen))
    {
        m_pElf_Ehdr = (Elf32_Ehdr* )m_pFileData;
        if(m_pElf_Ehdr->e_phoff > 0 && m_pElf_Ehdr->e_phentsize > 0 && m_pElf_Ehdr->e_phoff < dwDatalen)
        {
            m_pElf_Phdr = (Elf32_Phdr*)(m_pFileData + m_pElf_Ehdr->e_phoff);
        }
        if(m_pElf_Ehdr->e_shoff > 0 && m_pElf_Ehdr->e_shentsize > 0 && m_pElf_Ehdr->e_shoff < dwDatalen)
        {
            m_pElf_Shdr = (Elf32_Shdr*)(m_pFileData + m_pElf_Ehdr->e_shoff);
            if(m_pElf_Ehdr->e_shstrndx < m_pElf_Ehdr->e_shnum)
            {
                m_pElf_SstrTbl = (char*)m_pFileData + m_pElf_Shdr[m_pElf_Ehdr->e_shstrndx].sh_offset;
                m_nSStrTblLen = m_pElf_Shdr[m_pElf_Ehdr->e_shstrndx].sh_size;
            }
        }
    }
    
    return TRUE;
}

BOOL CElfParser::CloseFile()
{
    if(m_pFileData)
    {
        free(m_pFileData);
        m_pFileData = NULL;
        m_pElf_Ehdr = NULL;
        m_pElf_Phdr = NULL;
        m_pElf_Shdr = NULL;
        m_pElf_SstrTbl = NULL;
        m_nSStrTblLen = 0;
        m_fileType = COFF_TYPE_ELF;
    }

    return TRUE;
}

BOOL CElfParser::ShowFileTree(CTreeCtrl *pFileTree)
{
    TV_INSERTSTRUCT TreeCtrlItem;
    char chTemp[100];
    
    if(!m_pElf_Ehdr)
    {
        return FALSE;
    }
    
    // ELF 文件头
    TreeCtrlItem.hParent = TVI_ROOT;
    TreeCtrlItem.hInsertAfter = TVI_LAST;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
    TreeCtrlItem.item.pszText = _T("ELF Header");
    TreeCtrlItem.item.lParam = ELF_TABLE_HEADER;
    HTREEITEM hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);
    
    // 程序头表
    if( m_pElf_Phdr)
    {
        TreeCtrlItem.hParent = TVI_ROOT;
        TreeCtrlItem.hInsertAfter = TVI_LAST;
        TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
        TreeCtrlItem.item.pszText = _T("程序表");
        TreeCtrlItem.item.lParam = ELF_TABLE_PROGRAM;
        hTreeItem0 = pFileTree->InsertItem(&TreeCtrlItem);
        for(int i = 1; i <= m_pElf_Ehdr->e_phnum; i++)
        {
            TreeCtrlItem.hParent = hTreeItem0;
            sprintf(chTemp, "program header %03d", i);
            TreeCtrlItem.item.pszText = chTemp;
            TreeCtrlItem.item.lParam = ELF_TABLE_PROGRAM+i;
            pFileTree->InsertItem(&TreeCtrlItem);
        }
    }
    
    // 段表
    TreeCtrlItem.hParent = TVI_ROOT;
    TreeCtrlItem.hInsertAfter = TVI_LAST;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
    TreeCtrlItem.item.pszText = _T("段表");
    TreeCtrlItem.item.lParam = ELF_TABLE_SECTION;
    HTREEITEM hTreeItem1 = pFileTree->InsertItem(&TreeCtrlItem);
    for(int i = 1; i <= m_pElf_Ehdr->e_shnum; i++)
    {
        TreeCtrlItem.hParent = hTreeItem1;
        sprintf(chTemp, "section header %03d", i);
        TreeCtrlItem.item.pszText = chTemp;
        TreeCtrlItem.item.lParam = ELF_TABLE_SECTION+i;
        pFileTree->InsertItem(&TreeCtrlItem);
    }
    
    // 重定位表
    TreeCtrlItem.hParent = TVI_ROOT;
    TreeCtrlItem.hInsertAfter = TVI_LAST;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
    TreeCtrlItem.item.pszText = _T("重定位表");
    TreeCtrlItem.item.lParam = ELF_TABLE_RELOCATION;
    HTREEITEM hTreeItem2 = pFileTree->InsertItem(&TreeCtrlItem);
    
    // 字符串表
    TreeCtrlItem.hParent = TVI_ROOT;
    TreeCtrlItem.hInsertAfter = TVI_LAST;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
    TreeCtrlItem.item.pszText = _T("字符串表");
    TreeCtrlItem.item.lParam = ELF_TABLE_STRING;
    HTREEITEM hTreeItem3 = pFileTree->InsertItem(&TreeCtrlItem);
    
    // 段字符串表
    TreeCtrlItem.hParent = TVI_ROOT;
    TreeCtrlItem.hInsertAfter = TVI_LAST;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
    TreeCtrlItem.item.pszText = _T("段字符串表");
    TreeCtrlItem.item.lParam = ELF_TABLE_SECTSTR;
    HTREEITEM hTreeItem4 = pFileTree->InsertItem(&TreeCtrlItem);
    
    // 符号表
    TreeCtrlItem.hParent = TVI_ROOT;
    TreeCtrlItem.hInsertAfter = TVI_LAST;
    TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;
    TreeCtrlItem.item.pszText = _T("符号表");
    TreeCtrlItem.item.lParam = ELF_TABLE_SYMBOL;
	HTREEITEM hTreeItem5 = pFileTree->InsertItem(&TreeCtrlItem);

    return TRUE;
}

BOOL CElfParser::UpdateLstCtrl(CReportCtrl *pfileRepList, NM_TREEVIEW* pNMTreeView)
{
    int nRow;
    DWORD dwAddr = 0;
    char chText[500];
    char chTmp[10];
    int i, nRowCnt;

    if(!m_pElf_Ehdr)
    {
        return FALSE;
    }

    pfileRepList->DeleteAllItems();
    pfileRepList->DeleteAllColumns();
    switch(pNMTreeView->itemNew.lParam)
    {
    case ELF_TABLE_HEADER:
        // 插入相关列
        pfileRepList->InsertColumn(0, "地址", LVCFMT_LEFT, 60);
        pfileRepList->InsertColumn(1, "大小", LVCFMT_LEFT, 40);
        pfileRepList->InsertColumn(2, "十六进制取值", LVCFMT_LEFT, 120);
        pfileRepList->InsertColumn(3, "含义", LVCFMT_LEFT, 120);

        // 依次设置行内容
        dwAddr = 0;
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(0, chText);                         // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_ident));                // 设置大小列内容
        pfileRepList->SetItemText(nRow, 1, chText);
        memset(chText, 0, sizeof(chText));
        for(i = 0; i < sizeof(m_pElf_Ehdr->e_ident); i++)
        {
            sprintf(chTmp, "%02x", m_pElf_Ehdr->e_ident[i]);
            strcat(chText, chTmp);
        }
        pfileRepList->SetItemText(nRow, 2, chText);                         // 设置取值列内容 魔数
        sprintf(chText, "%s", "Magic为ELF(0x45 0x4c 0x46)");
        pfileRepList->SetItemText(nRow, 3, chText);                         // 设置含义列内容

        dwAddr += sizeof(m_pElf_Ehdr->e_ident);                             // 文件类型
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(1, chText);                         // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_type));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pElf_Ehdr->e_type);
        pfileRepList->SetItemText(nRow, 2, chText);
        if(ELF_FILE_REL == m_pElf_Ehdr->e_type)
            sprintf(chText, "%s", "可重定位文件");
        else if(ELF_FILE_EXEC == m_pElf_Ehdr->e_type)
            sprintf(chText, "%s", "可执行文件");
        else if(ELF_FILE_DYN == m_pElf_Ehdr->e_type)
            sprintf(chText, "%s", "共享目标文件");
        else
            sprintf(chText, "%s", "未知类型ELF文件");
        pfileRepList->SetItemText(nRow, 3, chText);

        dwAddr += sizeof(m_pElf_Ehdr->e_type);                              // 机器类型
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(2, chText);                         // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_machine));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pElf_Ehdr->e_machine);
        pfileRepList->SetItemText(nRow, 2, chText);
        if(m_pElf_Ehdr->e_machine < sizeof(sz_desc_e_machine)/sizeof(sz_desc_e_machine[0]))
            pfileRepList->SetItemText(nRow, 3, sz_desc_e_machine[m_pElf_Ehdr->e_machine]);
        else
            pfileRepList->SetItemText(nRow, 3, "Unknown");

        dwAddr += sizeof(m_pElf_Ehdr->e_machine);                           // ELF版本
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(3, chText);                         // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_version));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pElf_Ehdr->e_version);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "ELF 版本号");

        dwAddr += sizeof(m_pElf_Ehdr->e_version);                           // 入口地址
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(4, chText);                         // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_entry));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pElf_Ehdr->e_entry);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "ELF 入口地址");

        dwAddr += sizeof(m_pElf_Ehdr->e_entry);                             // 程序头表偏移
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(5, chText);                         // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_phoff));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pElf_Ehdr->e_phoff);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "程序头偏移");

        dwAddr += sizeof(m_pElf_Ehdr->e_phoff);                             // 段表头偏移
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(6, chText);                         // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_shoff));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pElf_Ehdr->e_shoff);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "段表头偏移");

        dwAddr += sizeof(m_pElf_Ehdr->e_shoff);                             // ELF标志位
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(7, chText);                         // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_flags));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%08x", m_pElf_Ehdr->e_flags);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "ELF标志位");

        dwAddr += sizeof(m_pElf_Ehdr->e_flags);                             // ELF文件头大小
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(8, chText);                         // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_ehsize));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pElf_Ehdr->e_ehsize);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "ELF文件头大小");

        dwAddr += sizeof(m_pElf_Ehdr->e_ehsize);                            // 程序头大小
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(9, chText);                         // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_phentsize));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pElf_Ehdr->e_phentsize);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "程序头大小");

        dwAddr += sizeof(m_pElf_Ehdr->e_phentsize);                         // 程序头个数
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(10, chText);                        // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_phnum));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pElf_Ehdr->e_phnum);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "程序头个数");

        dwAddr += sizeof(m_pElf_Ehdr->e_phnum);                             // 段表大小
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(11, chText);                        // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_shentsize));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pElf_Ehdr->e_shentsize);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "段表头大小");

        dwAddr += sizeof(m_pElf_Ehdr->e_shentsize);                         // 段表个数
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(12, chText);                        // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_shnum));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pElf_Ehdr->e_shnum);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "段表个数");

        dwAddr += sizeof(m_pElf_Ehdr->e_shnum);                             // 字符串表下标
        sprintf(chText, "%08x", dwAddr);
        nRow = pfileRepList->InsertItem(13, chText);                        // 插入行
        sprintf(chText, "%d", sizeof(m_pElf_Ehdr->e_shstrndx));
        pfileRepList->SetItemText(nRow, 1, chText);
        sprintf(chText, "%04x", m_pElf_Ehdr->e_shstrndx);
        pfileRepList->SetItemText(nRow, 2, chText);
        pfileRepList->SetItemText(nRow, 3, "字符串表在段表中的下标");
		break;
    case ELF_TABLE_PROGRAM:
        pfileRepList->InsertColumn(0, "说明", LVCFMT_LEFT, 400);
        nRow = pfileRepList->InsertItem(0, "可执行文件和共享目标文件包含该头");
        break;
    case ELF_TABLE_SECTION:
        pfileRepList->InsertColumn(0, "说明", LVCFMT_LEFT, 400);
        nRow = pfileRepList->InsertItem(0, "段表描述了ELF各个段的段名、段长度、在文件中的偏移、读写权限等信息");
        break;
    case ELF_TABLE_RELOCATION:
        break;
    case ELF_TABLE_STRING:
        if(m_pElf_Shdr)
        {
            pfileRepList->InsertColumn(0, "字符串下标值", LVCFMT_LEFT, 100);
            pfileRepList->InsertColumn(1, "字符串值", LVCFMT_LEFT, 300);
            pfileRepList->InsertColumn(2, "字符表名称", LVCFMT_LEFT, 100);
            for(i = 0; i < m_pElf_Ehdr->e_shnum; i++)
            {
                if(m_pElf_Shdr[i].sh_type == SHT_STRTAB && i != m_pElf_Ehdr->e_shstrndx)
                {
                    char* pStrTbl = (char*)m_pFileData + m_pElf_Shdr[i].sh_offset;
                    char* pString = pStrTbl;
                    nRowCnt = 0;
                    while(pString - pStrTbl < m_pElf_Shdr[i].sh_size)
                    {
                        sprintf(chText, "%08x", pString - pStrTbl);
                        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                        pfileRepList->SetItemText(nRow, 1, pString);
                        pfileRepList->SetItemText(nRow, 2, m_pElf_SstrTbl+m_pElf_Shdr[i].sh_name);
                        pString += (strlen(pString)+1);
                    }
                }
            }
        }
        break;
    case ELF_TABLE_SECTSTR:
        if(m_pElf_SstrTbl)
        {
            /*char* pString = m_pElf_SstrTbl;
            pfileRepList->InsertColumn(0, "字符串值", LVCFMT_LEFT, 200);
            nRowCnt = 0;
            while(pString - m_pElf_SstrTbl < m_nSStrTblLen)
            {
                pfileRepList->InsertItem(nRowCnt++, pString);
                pString += (strlen(pString)+1);
            }*/
            pfileRepList->InsertColumn(0, "字符串下标值", LVCFMT_LEFT, 100);
            pfileRepList->InsertColumn(1, "字符串值", LVCFMT_LEFT, 300);
            pfileRepList->InsertColumn(2, "字符表名称", LVCFMT_LEFT, 100);
            char* pString = m_pElf_SstrTbl;
            nRowCnt = 0;
            while(pString - m_pElf_SstrTbl < m_nSStrTblLen)
            {
                sprintf(chText, "%08x", pString - m_pElf_SstrTbl);
                nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                pfileRepList->SetItemText(nRow, 1, pString);
                pfileRepList->SetItemText(nRow, 2, m_pElf_SstrTbl+m_pElf_Shdr[m_pElf_Ehdr->e_shstrndx].sh_name);
                pString += (strlen(pString)+1);
            }
        }
        break;
    case ELF_TABLE_SYMBOL:
        if(m_pElf_Shdr)
        {
            pfileRepList->InsertColumn(0, "符号名", LVCFMT_LEFT, 100);
            pfileRepList->InsertColumn(1, "符号取值", LVCFMT_LEFT, 100);
            pfileRepList->InsertColumn(2, "符号大小", LVCFMT_LEFT, 100);
            pfileRepList->InsertColumn(3, "符号类型和绑定信息", LVCFMT_LEFT, 200);
            pfileRepList->InsertColumn(4, "st_other", LVCFMT_LEFT, 50);
            pfileRepList->InsertColumn(5, "符号所在段", LVCFMT_LEFT, 100);
            for(i = 0; i < m_pElf_Ehdr->e_shnum; i++)
            {
                if(m_pElf_Shdr[i].sh_type == SHT_SYMTAB)
                {
                    /*char* pStrTbl = (char*)m_pFileData + m_pElf_Shdr[i].sh_offset;
                    char* pString = pStrTbl;
                    nRowCnt = 0;
                    while(pString - pStrTbl < m_pElf_Shdr[i].sh_size)
                    {
                        sprintf(chText, "%08x", pString - pStrTbl);
                        nRow = pfileRepList->InsertItem(nRowCnt++, chText);
                        pfileRepList->SetItemText(nRow, 1, pString);
                        pfileRepList->SetItemText(nRow, 2, m_pElf_SstrTbl+m_pElf_Shdr[i].sh_name);
                        pString += (strlen(pString)+1);
                    }*/
                }
            }
        }
        break;
    default:
        if(pNMTreeView->itemNew.lParam>ELF_TABLE_PROGRAM && pNMTreeView->itemNew.lParam<ELF_TABLE_SECTION)
        {
            // 插入相关列
            pfileRepList->InsertColumn(0, "地址", LVCFMT_LEFT, 60);
            pfileRepList->InsertColumn(1, "大小", LVCFMT_LEFT, 40);
            pfileRepList->InsertColumn(2, "十六进制取值", LVCFMT_LEFT, 100);
            pfileRepList->InsertColumn(3, "含义", LVCFMT_LEFT, 300);

            int nIndex = pNMTreeView->itemNew.lParam - ELF_TABLE_PROGRAM - 1;
            // 依次设置行内容
            if(nIndex < m_pElf_Ehdr->e_phnum)
            {
                nRowCnt = 0;
                dwAddr = m_pElf_Ehdr->e_phoff + nIndex*sizeof(Elf32_Phdr);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Phdr[nIndex].p_type));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Phdr[nIndex].p_type);
                pfileRepList->SetItemText(nRow, 2, chText);
                if(m_pElf_Phdr[nIndex].p_type < sizeof(sz_desc_p_type)/sizeof(sz_desc_p_type[0]))
                    pfileRepList->SetItemText(nRow, 3, sz_desc_p_type[m_pElf_Phdr[nIndex].p_type]);
                else
                    pfileRepList->SetItemText(nRow, 3, "Unknown Segment type");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Phdr[nIndex].p_type);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Phdr[nIndex].p_offset));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Phdr[nIndex].p_offset);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "Segment在文件中的偏移");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Phdr[nIndex].p_offset);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Phdr[nIndex].p_vaddr));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Phdr[nIndex].p_vaddr);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "Segment第一个字节在进程虚拟地址空间起始位置");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Phdr[nIndex].p_vaddr);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Phdr[nIndex].p_paddr));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Phdr[nIndex].p_paddr);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "Segment物理装载地址");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Phdr[nIndex].p_paddr);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Phdr[nIndex].p_filesz));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Phdr[nIndex].p_filesz);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "Segment在ELF文件中所占空间大小");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Phdr[nIndex].p_filesz);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Phdr[nIndex].p_memsz));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Phdr[nIndex].p_memsz);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "Segment在进程虚拟地址空间所占用空间大小");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Phdr[nIndex].p_memsz);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Phdr[nIndex].p_flags));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Phdr[nIndex].p_flags);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "Segment权限属性");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Phdr[nIndex].p_flags);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Phdr[nIndex].p_align));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Phdr[nIndex].p_align);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "Segment对齐属性");
            }
        }
        else if(pNMTreeView->itemNew.lParam>ELF_TABLE_SECTION && pNMTreeView->itemNew.lParam<ELF_TABLE_RELOCATION)
        {
            // 插入相关列
            pfileRepList->InsertColumn(0, "地址", LVCFMT_LEFT, 60);
            pfileRepList->InsertColumn(1, "大小", LVCFMT_LEFT, 40);
            pfileRepList->InsertColumn(2, "十六进制取值", LVCFMT_LEFT, 100);
            pfileRepList->InsertColumn(3, "含义", LVCFMT_LEFT, 300);
            
            int nIndex = pNMTreeView->itemNew.lParam - ELF_TABLE_SECTION - 1;
            // 依次设置行内容
            if(nIndex < m_pElf_Ehdr->e_shnum)
            {
                nRowCnt = 0;
                dwAddr = m_pElf_Ehdr->e_shoff + nIndex*sizeof(Elf32_Shdr);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Shdr[nIndex].sh_name));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Shdr[nIndex].sh_name);
                pfileRepList->SetItemText(nRow, 2, chText);
                if(m_pElf_SstrTbl && m_pElf_Shdr[nIndex].sh_name < m_nSStrTblLen)
                {
                    sprintf(chText, "段名字符串(%s)在段字符串表中的偏移", m_pElf_SstrTbl+m_pElf_Shdr[nIndex].sh_name);
                    pfileRepList->SetItemText(nRow, 3, chText);
                }
                else
                    pfileRepList->SetItemText(nRow, 3, "段名字符串在段字符串表中的偏移");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Shdr[nIndex].sh_name);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Shdr[nIndex].sh_type));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Shdr[nIndex].sh_type);
                pfileRepList->SetItemText(nRow, 2, chText);
                if(m_pElf_Shdr[nIndex].sh_type < sizeof(sz_desc_sh_type)/sizeof(sz_desc_sh_type[0]))
                {
                    sprintf(chText, "段类型：%s", sz_desc_sh_type[m_pElf_Shdr[nIndex].sh_type]);
                    pfileRepList->SetItemText(nRow, 3, chText);
                }
                else
                    pfileRepList->SetItemText(nRow, 3, "未知段类型");
                
                nRowCnt++;
                dwAddr += sizeof(m_pElf_Shdr[nIndex].sh_type);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Shdr[nIndex].sh_flags));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Shdr[nIndex].sh_flags);
                pfileRepList->SetItemText(nRow, 2, chText);
                if(m_pElf_Shdr[nIndex].sh_flags < sizeof(sz_desc_sh_flags)/sizeof(sz_desc_sh_flags[0]))
                {
                    sprintf(chText, "段属性：%s", sz_desc_sh_flags[m_pElf_Shdr[nIndex].sh_flags]);
                    pfileRepList->SetItemText(nRow, 3, chText);
                }
                else
                {
                    pfileRepList->SetItemText(nRow, 3, "段属性未知");
                }

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Shdr[nIndex].sh_flags);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Shdr[nIndex].sh_addr));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Shdr[nIndex].sh_addr);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "段虚拟地址");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Shdr[nIndex].sh_addr);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Shdr[nIndex].sh_offset));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Shdr[nIndex].sh_offset);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "段偏移");
                
                nRowCnt++;
                dwAddr += sizeof(m_pElf_Shdr[nIndex].sh_offset);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Shdr[nIndex].sh_size));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Shdr[nIndex].sh_size);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "段的长度");
                
                nRowCnt++;
                dwAddr += sizeof(m_pElf_Shdr[nIndex].sh_size);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Shdr[nIndex].sh_link));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Shdr[nIndex].sh_link);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "段链接信息");
                
                nRowCnt++;
                dwAddr += sizeof(m_pElf_Shdr[nIndex].sh_link);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Shdr[nIndex].sh_info));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Shdr[nIndex].sh_info);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "段链接信息");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Shdr[nIndex].sh_info);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Shdr[nIndex].sh_addralign));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Shdr[nIndex].sh_addralign);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "段地址对齐");

                nRowCnt++;
                dwAddr += sizeof(m_pElf_Shdr[nIndex].sh_addralign);
                sprintf(chText, "%08x", dwAddr);                                        // 插入行
                nRow = pfileRepList->InsertItem(nRowCnt, chText);                       // 地址
                sprintf(chText, "%d", sizeof(m_pElf_Shdr[nIndex].sh_entsize));
                pfileRepList->SetItemText(nRow, 1, chText);                             // 设置大小列内容
                sprintf(chText, "%08x", m_pElf_Shdr[nIndex].sh_entsize);
                pfileRepList->SetItemText(nRow, 2, chText);
                pfileRepList->SetItemText(nRow, 3, "Section Entry Size项的长度");
            }
        }
        break;
    }
    return TRUE;
}