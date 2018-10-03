// NOTE: DOS header stuff.
#define IMAGE_DOS_SIGNATURE            0x5A4D     // MZ

// NOTE: PE header stuff.
#define IMAGE_NT_SIGNATURE             0x00004550 // PE00

/* Machine Types */
#define IMAGE_FILE_MACHINE_AMD64       0x8664
#define IMAGE_FILE_MACHINE_ARM         0x1c0
#define IMAGE_FILE_MACHINE_ARM64       0xaa64
#define IMAGE_FILE_MACHINE_I386        0x14c

/* Characteristics */
#define IMAGE_FILE_RELOCS_STRIPPED     0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE    0x0002
#define IMAGE_FILE_LINE_NUMS_STRIPPED  0x0004
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED 0x0008
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020
#define IMAGE_FILE_DEBUG_STRIPPED 	 0x0200

/* Optional Header Magic Number */
#define PE32                           0x10b  // PE32, 32-bit.
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC  0x020b // PE32+, 64-bit.

/* Windows Subsystem */
#define IMAGE_SUBSYSTEM_WINDOWS_GUI    0x0002
#define IMAGE_SUBSYSTEM_WINDOWS_CUI    0x0003

// NOTE: Section header characteristics.
#define IMAGE_SCN_CNT_CODE 	          0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA   0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define IMAGE_SCN_MEM_EXECUTE 	       0x20000000 
#define IMAGE_SCN_MEM_READ           	0x40000000

#pragma pack(push, 1)

typedef struct DOS_Header {
    u16 signature;              // NOTE: MZ
    u16 last_page_size;
    u16 page_count;
    u16 relocation_count;
    u16 paragraph_header_count;
    u16 minalloc;
    u16 maxalloc;
    u16 ss;
    u16 sp;
    u16 checksum;
    u16 ip;
    u16 cs;
    u16 relocpos;
    u16 noverlay;
    u16 reserved1[4];
    u16 oem_id;
    u16 oem_info;
    u16 reserved2[10];
    u32 pe_header_offset;      // NOTE: Always at 0x3c or 60 bytes into the file.
}
DOS_Header;

typedef struct COFF_Header
{
    u16 machine;                 // CPU type. Check the Machine Types macros above.
    u16 section_count;           // Number of sections in the section table.
    u32 time_date_stamp;         // Low 32 bits of seconds since epoch.
    u32 pointer_to_symbol_table; // Should be zero.
    u32 symbol_count;            // Should be zero.
    u16 optional_header_size;    // The size of the optional header. Not fixed.
    u16 characteristics;         // Check the Characteristics macros above.
}
COFF_Header;

// NOTE: It's called optional, but it's required for image files.
typedef struct Optional_Header
{
    // NOTE: The first eight are standard COFF fields.
    u16 signature;               // "Magic number." Check macros above.
    u8  major_linker_version;    // Zero, because we don't have a linker!
    u8  minor_linker_version;    // Zero, because we don't have a linker!
    u32 code_size;               // Size of "text" section, or sum if multiple.
    u32 initialized_data_size;   // Size of data section(s).
    u32 uninitialized_data_size; // Size of uninitialized data (BSS) section(s).
    u32 address_of_entry_point;  // RVA (to image base) of first instruction.
    u32 base_of_code;            // RVA of beginning of code section.
    // NOTE: u32 base_of_data exists in PE32 images.
    
    // NOTE: These are Windows-specific fields.
    u64 image_base;              // 32-bit value in PE32. Default is 0x00400000.
    u32 section_alignment;       // Byte-alignment of sections. Default is page size.
    u32 file_alignment;          // Byte-alignment 
    u16 major_os_version;        // 5 for XP.
    u16 minor_os_version;        // 1 for XP.
    u16 major_image_version;     //
    u16 minor_image_version;     //
    u16 major_subsystem_version; //
    u16 minor_subsystem_version; //
    u32 win32_version_value;     // Reserved, must be zero.
    u32 image_size;              // Including all headers. Mult of section_alignment.
    u32 headers_size;            // Rounded up to mult of file_alignment.
    u32 check_sum;               // TODO: Gotta figure this one out...
    u16 subsystem;               // Check Windows Subsystem macros above.
    u16 dll_characteristics;     // TODO: We'll want to build DLLs at some point!
    u64 stack_reserve_size;      // 32-bit in PE32.
    u64 stack_commit_size;       // 32-bit in PE32.
    u64 heap_reserve_size;       // 32-bit in PE32.
    u64 heap_commit_size;        // 32-bit in PE32.
    u32 loader_flags;            // Reserved, must be zero.
    u32 data_directory_count;    // Number of data directories at end of opt. header.
}
Optional_Header;

typedef struct Data_Directory
{
    u32 rva;  // Address of table relative to image base address.
    u32 size; // Size in bytes.
}
Data_Directory;

typedef struct Section_Header
{
    u8  name[8];                // UTF-8 encoded string.
    u32 virtual_size;           // Size of section when loaded into memory.
    u32 virtual_address;        // Multiple of optional header's section_alignment.
    u32 raw_data_size;          // Multiple of optional header's file_alignment.
    u32 pointer_to_raw_data;    // Multiple of the file_alignment.
    u32 pointer_to_relocations;
    u32 pointer_to_linenumbers;
    u16 relocation_count;
    u16 linenumber_count;
    u32 characteristics;
}
Section_Header;

#pragma pack(pop)

typedef struct EXE_Options
{
    char company_name[256];
    char product_name[256];
    char version[256];
    char copyright[256];
}
EXE_Options

u32 compiler_format_exe(void *buffer, EXE_Options options)
{
    DOS_Header *dos_header = (DOS_Header *)buffer;
    dos_header->signature        = IMAGE_DOS_SIGNATURE;
    dos_header->pe_header_offset = sizeof(DOS_Header);
    
    u32 *pe_signature = (u32 *)(dos_header + 1);
    *pe_signature = IMAGE_NT_SIGNATURE;
    
    COFF_Header *coff_header = (COFF_Header *)(pe_signature + 1);
    
    u32 size = 0;
    
    int section_count = 1;
    int section_alignment = 4096;
    int file_alignment = 512;
    
    coff_header->machine              = IMAGE_FILE_MACHINE_AMD64;
    coff_header->section_count        = 1;
    coff_header->optional_header_size = sizeof(Optional_Header);
    coff_header->characteristics      = IMAGE_FILE_LARGE_ADDRESS_AWARE | IMAGE_FILE_EXECUTABLE_IMAGE;
    
    Optional_Header *optional_header = (Optional_Header *)(coff_header + 1);
    
    int total_headers_size =
        sizeof(DOS_Header) +
        sizeof(pe_signature) +
        sizeof(COFF_Header) +
        (sizeof(Data_Directory)*optional_header->data_directory_count) +
        (sizeof(Section_Header)*section_count);
    
    optional_header->signature = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    //pe_header->optional_header.code_size = file_alignment;
    //pe_header->optional_header.initialized_data_size = file_alignment;
    optional_header->address_of_entry_point = section_alignment;// TODO: Will this ever NOT be the beginning of the first section?
    // pe_header->optional_header.base_of_code = section_alignment;
    optional_header->image_base = 5368709120;
    optional_header->section_alignment = section_alignment;
    optional_header->file_alignment = file_alignment;
    //pe_header->optional_header.major_operating_system_version = 6; // NOTE: Do we need this?
    optional_header->major_subsystem_version = 6;
    optional_header->image_size = section_alignment*2; // TODO: Add in the actual size of the sections.
    // NOTE: Needs to a multiple of section_alignment.
    optional_header->headers_size = file_alignment*2;
    optional_header->subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    optional_header->stack_reserve_size = 100000;
    optional_header->stack_commit_size = 1000;
    optional_header->heap_reserve_size = 100000;
    optional_header->heap_commit_size = 1000;
    
#if 0
    Section_Header *section_header = (Section_Header *)((optional_header + 1));
    section_header->name[0] = '.';
    section_header->name[1] = 't';
    section_header->name[2] = 'e';
    section_header->name[3] = 'x';
    section_header->name[4] = 't';
    section_header->virtual_size = 6;
    section_header->virtual_address = section_alignment; // TODO: This should be multiplied by the index of this section. For now, since we only have one section, we don't care.
    section_header->raw_data_size = file_alignment;
    section_header->pointer_to_raw_data = file_alignment * 2;//total_headers_size + (file_alignment);
    section_header->characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
    
    u8 *code = (u8 *)buffer + file_alignment * 2;
    
#define REX_W         0x48
#define SUB_RM16_IMM8 0x83
    
    *code++ = REX_W;
    *code++ = SUB_RM16_IMM8;
    *code++ = 0x00;
    *code++ = 0x28; // 40 bytes. Aligning stack pointer?
    *code++ = 0x00;
    *code++ = 0xc3;
    
    size = file_alignment*3;
#endif
    
    return size;
}
