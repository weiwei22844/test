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
    //	该成员给出了该段的驻留位置相对于文件开始处的偏移。
    Elf32_Addr p_vaddr;
    //	该成员给出了该段在内存中的首字节地址。
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    //	文件映像中该段的字节数；它可能是 0
    Elf32_Word p_memsz;
    //	内存映像中该段的字节数；它可能是 0
    Elf32_Word p_flags;
    Elf32_Word p_align;
    //	该成员给出了该段在内存和文件中排列值。
    //	0 和 1 表示不需要排列。否则， p_align 必须为正的 2 的幂，
    //	并且 p_vaddr 应当等于 p_offset 模 p_align
} Elf32_Phdr;           // 32 byte

typedef struct {
    // 该成员指定了这个section的名字。
    // 它的值是section报头字符表section的索引。[看以下的“String Table”], 以NULL空字符结束。
    Elf32_Word sh_name;
    
    //	Section Types, sh_type
    //	---------------------------
    //	Name                 Value    Description
    //	====                 =====    ===========
    //	SHT_NULL				 0    该值表明该section头是无效的；它没有相关的section。
    //	SHT_PROGBITS			 1    该section保存被程序定义了的一些信息，它的格式和意义取决于程序本身。
    //	SHT_SYMTAB				 2    该sections保存着一个符号表（symbol table）。
    //	SHT_STRTAB				 3    该section保存着一个字符串表。
    //	SHT_RELA				 4    该section保存着具有明确加数的重定位入口。
    //	SHT_HASH				 5    该标号保存着一个标号的哈希(hash)表。
    //	SHT_DYNAMIC				 6    该section保存着动态连接的信息。
    //	SHT_NOTE				 7    该section保存着其他的一些标志文件的信息。
    //	SHT_NOBITS				 8    该类型的section在文件中不占空间，但是类似SHT_PROGBITS。
    //	SHT_REL					 9    该section保存着重定位的入口。
    //	SHT_SHLIB				10    该section类型保留但语意没有指明。包含这个类型的section的程序是不符合ABI的。
    //	SHT_DYNSYM				11    该sections保存着一个符号表（symbol table）。
    //	SHT_LOPROC		0x70000000    在这范围之间的值为特定处理器语意保留的。
    //	SHT_HIPROC		0x7fffffff    在这范围之间的值为特定处理器语意保留的。
    //	SHT_LOUSER		0x80000000    该变量为应用程序保留的索引范围的最小边界。
    //	SHT_HIUSER		0xffffffff    该变量为应用程序保留的索引范围的最大边界。
    Elf32_Word sh_type;

    //	Section Attribute Flags, sh_flags
    //	-----------------------------------		
    //	Name                Value    Description
    //	====                =====    ===========
    //	SHF_WRITE             0x1    该section包含了在进程执行过程中可被写的数据。
    //	SHF_ALLOC             0x2    该section在进程执行过程中占据着内存。
    //	SHF_EXECINSTR         0x4    该section包含了可执行的机器指令。
    //	SHF_MASKPROC   0xf0000000    所有的包括在这掩码中的位为特定处理语意保留的。
    Elf32_Word sh_flags;
    
    //	假如该section将出现在进程的内存映象空间里，该成员给出了一个该section在内存中的位置。否则，该变量为0。
    Elf32_Addr sh_addr;
    
    //	该成员变量给出了该section的字节偏移量(从文件开始计数)。
    Elf32_Off  sh_offset;

    //	该成员给你了section的字节大小。
    Elf32_Word sh_size;
    
    //	该成员保存了一个section报头表的索引连接，它的解释依靠该section的类型。
    //	更多信息参见表"sh_link and sh_info Interpretation"
    Elf32_Word sh_link;
    
    //	该成员保存着额外的信息，它的解释依靠该section的类型。
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
    
    //	一些sections有地址对齐的约束。
    Elf32_Word sh_addralign;
    
    //	一些sections保存着一张固定大小入口的表，就象符号表。
    //	对于这样一个section来说，该成员给出了每个入口的字节大小。
    //	如果该section没有保存着一张固定大小入口的表，该成员就为0。
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
