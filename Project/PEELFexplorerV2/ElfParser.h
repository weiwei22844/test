// ElfParser.h: interface for the CElfParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ELFPARSER_H__BFEAB614_E57E_4F36_BFD7_832EE3DF251E__INCLUDED_)
#define AFX_ELFPARSER_H__BFEAB614_E57E_4F36_BFD7_832EE3DF251E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CoffParser.h"
#include "elfEx.h"

typedef	UINT	Elf32_Addr;
typedef	WORD	Elf32_Half;
typedef	UINT	Elf32_Off;
typedef	INT	    Elf32_Sword;
typedef	UINT	Elf32_Word;

/*typedef struct{
    unsigned char e_ident[16];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
}Elf32_Ehdr;            // 52 byte

typedef struct {
    Elf32_Word p_type;
    //	Name             Value
    //	====             =====
    //	PT_NULL              0
    //	PT_LOAD              1
    //	PT_DYNAMIC           2
    //	PT_INTERP            3
    //	PT_NOTE              4
    //	PT_SHLIB             5
    //	PT_PHDR              6
    //	PT_LOPROC   0x70000000
    //	PT_HIPROC   0x7fffffff
    Elf32_Off  p_offset;
    //	�ó�Ա�����˸öε�פ��λ��������ļ���ʼ����ƫ�ơ�
    Elf32_Addr p_vaddr;
    //	�ó�Ա�����˸ö����ڴ��е����ֽڵ�ַ��
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    //	�ļ�ӳ���иöε��ֽ������������� 0
    Elf32_Word p_memsz;
    //	�ڴ�ӳ���иöε��ֽ������������� 0
    Elf32_Word p_flags;
    Elf32_Word p_align;
    //	�ó�Ա�����˸ö����ڴ���ļ�������ֵ��
    //	0 �� 1 ��ʾ����Ҫ���С����� p_align ����Ϊ���� 2 ���ݣ�
    //	���� p_vaddr Ӧ������ p_offset ģ p_align
} Elf32_Phdr;           // 32 byte

typedef struct {
    // �ó�Աָ�������section�����֡�
    // ����ֵ��section��ͷ�ַ���section��������[�����µġ�String Table��], ��NULL���ַ�������
    Elf32_Word sh_name;
    
    //	Section Types, sh_type
    //	---------------------------
    //	Name                 Value    Description
    //	====                 =====    ===========
    //	SHT_NULL				 0    ��ֵ������sectionͷ����Ч�ģ���û����ص�section��
    //	SHT_PROGBITS			 1    ��section���汻�������˵�һЩ��Ϣ�����ĸ�ʽ������ȡ���ڳ�����
    //	SHT_SYMTAB				 2    ��sections������һ�����ű�symbol table����
    //	SHT_STRTAB				 3    ��section������һ���ַ�����
    //	SHT_RELA				 4    ��section�����ž�����ȷ�������ض�λ��ڡ�
    //	SHT_HASH				 5    �ñ�ű�����һ����ŵĹ�ϣ(hash)��
    //	SHT_DYNAMIC				 6    ��section�����Ŷ�̬���ӵ���Ϣ��
    //	SHT_NOTE				 7    ��section������������һЩ��־�ļ�����Ϣ��
    //	SHT_NOBITS				 8    �����͵�section���ļ��в�ռ�ռ䣬��������SHT_PROGBITS��
    //	SHT_REL					 9    ��section�������ض�λ����ڡ�
    //	SHT_SHLIB				10    ��section���ͱ���������û��ָ��������������͵�section�ĳ����ǲ�����ABI�ġ�
    //	SHT_DYNSYM				11    ��sections������һ�����ű�symbol table����
    //	SHT_LOPROC		0x70000000    ���ⷶΧ֮���ֵΪ�ض����������Ᵽ���ġ�
    //	SHT_HIPROC		0x7fffffff    ���ⷶΧ֮���ֵΪ�ض����������Ᵽ���ġ�
    //	SHT_LOUSER		0x80000000    �ñ���ΪӦ�ó�������������Χ����С�߽硣
    //	SHT_HIUSER		0xffffffff    �ñ���ΪӦ�ó�������������Χ�����߽硣
    Elf32_Word sh_type;

    //	Section Attribute Flags, sh_flags
    //	-----------------------------------		
    //	Name                Value    Description
    //	====                =====    ===========
    //	SHF_WRITE             0x1    ��section�������ڽ���ִ�й����пɱ�д�����ݡ�
    //	SHF_ALLOC             0x2    ��section�ڽ���ִ�й�����ռ�����ڴ档
    //	SHF_EXECINSTR         0x4    ��section�����˿�ִ�еĻ���ָ�
    //	SHF_MASKPROC   0xf0000000    ���еİ������������е�λΪ�ض��������Ᵽ���ġ�
    Elf32_Word sh_flags;
    
    //	�����section�������ڽ��̵��ڴ�ӳ��ռ���ó�Ա������һ����section���ڴ��е�λ�á����򣬸ñ���Ϊ0��
    Elf32_Addr sh_addr;
    
    //	�ó�Ա���������˸�section���ֽ�ƫ����(���ļ���ʼ����)��
    Elf32_Off  sh_offset;

    //	�ó�Ա������section���ֽڴ�С��
    Elf32_Word sh_size;
    
    //	�ó�Ա������һ��section��ͷ����������ӣ����Ľ���������section�����͡�
    //	������Ϣ�μ���"sh_link and sh_info Interpretation"
    Elf32_Word sh_link;
    
    //	�ó�Ա�����Ŷ������Ϣ�����Ľ���������section�����͡�
    //	sh_link and sh_info Interpretation
    //	-------------------------------------------------------------------------------
    //	sh_type        sh_link                          sh_info
    //	=======        =======                          =======
    //	SHT_DYNAMIC    The section header index of      0
    //	               the string table used by
    //	               entries in the section.
    //	-------------------------------------------------------------------------------
    //	SHT_HASH       The section header index of      0
    //	               the symbol table to which the
    //	               hash table applies.
    //	-------------------------------------------------------------------------------
    //	SHT_REL        The section header index of      The section header index of
    //	SHT_RELA       the associated symbol table.     the section to which the
    //	                                                relocation applies.
    //	-------------------------------------------------------------------------------
    //	SHT_SYMTAB     The section header index of      One greater than the symbol
    //	-------------------------------------------------------------------------------
    //	SHT_DYNSYM     the associated string table.     table index of the last local
    //	               symbol (binding STB_LOCAL).
    //	-------------------------------------------------------------------------------
    //	other          SHN_UNDEF                        0
    //	-------------------------------------------------------------------------------
    Elf32_Word sh_info;
    
    //	һЩsections�е�ַ�����Լ����
    Elf32_Word sh_addralign;
    
    //	һЩsections������һ�Ź̶���С��ڵı�������ű�
    //	��������һ��section��˵���ó�Ա������ÿ����ڵ��ֽڴ�С��
    //	�����sectionû�б�����һ�Ź̶���С��ڵı��ó�Ա��Ϊ0��
    Elf32_Word sh_entsize;
} Elf32_Shdr;       // 40 byte*/

typedef enum
{
    ELF_TABLE_UNKOWN = 0,
    ELF_TABLE_HEADER = 1000,
    ELF_TABLE_PROGRAM = 2000,
    ELF_TABLE_SECTION = 3000,
    ELF_TABLE_RELOCATION = 4000,
    ELF_TABLE_STRING = 5000,
    ELF_TABLE_SECTSTR = 6000,
    ELF_TABLE_SYMBOL = 7000,
}ELF_TABLE_TYPE;

typedef enum
{
    ELF_FILE_UNKOWN = 0,
    ELF_FILE_REL,
    ELF_FILE_EXEC,
    ELF_FILE_DYN
}ELF_FILE_TYPE;

/*typedef enum{
    SHT_NULL = 0,
    SHT_PROGBITS,
    SHT_SYMTAB,
    SHT_STRTAB,
    SHT_RELA,
    SHT_HASH,
    SHT_DYNAMIC,
    SHT_NOTE,
    SHT_NOBITS,
    SHT_REL,
    SHT_SHLIB,
    SHT_DYNSYM
};*/

class CElfParser : public CCoffParser  
{
public:
	CElfParser();
	virtual ~CElfParser();

    virtual BOOL OpenFile(char *pName);
    virtual BOOL CloseFile();
    virtual BOOL ShowFileTree(CTreeCtrl *pFileTree);
    virtual BOOL UpdateLstCtrl(CReportCtrl *pfileRepList, NM_TREEVIEW* pNMTreeView);

public:
    BYTE *m_pFileData;
    Elf32_Ehdr* m_pElf_Ehdr;
    Elf32_Phdr* m_pElf_Phdr;
    Elf32_Shdr* m_pElf_Shdr;
    char* m_pElf_SstrTbl;
    int m_nSStrTblLen;
};

#endif // !defined(AFX_ELFPARSER_H__BFEAB614_E57E_4F36_BFD7_832EE3DF251E__INCLUDED_)
