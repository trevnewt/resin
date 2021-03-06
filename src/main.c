#include "basic.h"
#include "string.c"

#include "win32_memory.c"
#include "win32_io.c"

#include "resource.h"

void   __stdcall ExitProcess         (u32 exit_code);
char * __stdcall GetCommandLineA     (void);

typedef enum {
    THIS_FILENAME,
    EXE_FILENAME,
    ICON_FILENAME,
    DESCRIPTION,
    PRODUCT_NAME,
    VERSION,
    COPYRIGHT,
    
    ARG_COUNT
} CommandLineArg;

static u16 add_string_resource(u16 *dst, char *key, char *val)
{
    size key_len = str_len(key) + 1; // NOTE: +1 for \0 terminator.
    size val_len = str_len(val) + 1; // NOTE: +1 for \0 terminator.
    
    // NOTE: The first pad comes after three 16-bit values and the key. If the key has an even length, then we need a 16-bit pad to align the subsequent "val" array to a 32-bit boundary. The second pad comes after "val", which we just ensured will start at a 32-bit boundary. If "val" has an odd length, then we need a 16-bit pad to make sure we end on a 32-bit boundary.
    u8 pad1_len = (key_len % 2) ? 0 : 1;
    u8 pad2_len = (val_len % 2) ? 1 : 0;
    
    size total_size = sizeof(u16) * (3 + key_len + pad1_len + val_len + pad2_len);
    
    if (total_size <= MAX_u16)
    {
        dst[0] = (u16)total_size; // NOTE: Size of string resource block in BYTES.
        dst[1] = (u16)val_len;    // NOTE: Size of value in WORDS.
        dst[2] = 1;               // NOTE: Type. 0 for binary, 1 for text.
        
        str_cpy_char_to_wide(dst, key);
        str_cpy_char_to_wide(dst + key_len + pad1_len, val);
    }
    else
    {
        total_size = 0;
    }
    
    return (u16)total_size;
}

static void update_icon(void *exe_handle, char *icon_filename)
{
    Win32FileContents ico = win32_read_file(icon_filename);
    if (!ico.error)
    {
        ICONDIR *src_header = (ICONDIR *)ico.memory;
        ICONDIRENTRY *src_entries = (ICONDIRENTRY *)(src_header + 1);
        
        size bytes = sizeof(ICONDIR) + (src_header->image_count * sizeof(GRPICONDIRENTRY));
        ICONDIR *dst_header = (ICONDIR *)VirtualAlloc(0, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        dst_header->resource_type = src_header->resource_type;
        GRPICONDIRENTRY *dst_entries = (GRPICONDIRENTRY *)(dst_header + 1);
        
        for (u16 i = 0; i < src_header->image_count; ++i)
        {
            u16 id = 147 + i; // NOTE: Random constant.
            ICONDIRENTRY *src_entry = src_entries + i;
            u8 *image_data = (u8 *)ico.memory + src_entry->image_offset;
            
            // NOTE: Verify the actual width of the image data.
            u32 image_width;
            if (image_data[1] == 'P' && image_data[2] == 'N' && image_data[3] == 'G')
            {
                // NOTE: This icon is stored as a PNG. Windows recommends that 256x256 icons (the largest it displays) be stored in PNG format to avoid the large sizes that uncompressed BMP would incur, but any icon in a .ico file can be stored as a PNG. The width is located at a 16-byte offset, and must be byte-reversed - integers in PNG are in network byte order (big endian).
                u8 *png_width = image_data + 16;
                image_width = png_width[0] << 24 | png_width[1] << 16 | png_width[2] << 8 | png_width[3];
            }
            else
            {
                // NOTE: This icon is stored as a BMP (without its file header structure). The width is located at a 4-byte offset.
                image_width = *(u32 *)(image_data + 4);
            }
            
            // NOTE: If there are icons larger than 256x256 in the .ico file, we don't want to include them - they may cause bad behavior, and Windows doesn't display them anyway, so they would just bloat our executable.
            if (image_width <= 256)
            {
                UpdateResourceA(exe_handle, RT_ICON, MAKEINTRESOURCE(id), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), image_data, src_entry->size);
                
                GRPICONDIRENTRY *dst_entry = dst_entries + dst_header->image_count;
                
                dst_entry->width          = src_entry->width;
                dst_entry->height         = src_entry->height;
                dst_entry->color_count    = src_entry->color_count;
                dst_entry->color_planes   = src_entry->color_planes;
                dst_entry->bits_per_pixel = src_entry->bits_per_pixel;
                dst_entry->size           = src_entry->size;
                dst_entry->id             = id;
                
                dst_header->image_count += 1;
            }
        }
        
        u32 size = sizeof(ICONDIR) + (sizeof(GRPICONDIRENTRY) * dst_header->image_count);
        
        UpdateResourceA(exe_handle, RT_GROUP_ICON, "MAINICON", MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), dst_header, size);
    }
    else
    {
        char buf[256];
        win32_printf(buf, "Unable to find icon file: %\n", icon_filename);
    }
}

bool parse_command_line(char *args[], char *str)
{
    // NOTE: Rather than allocate new strings for each arg and copy them over, we just set our pointers to the starts of the args and replace delimiting spaces (and quotes) with null terminators.
    
    bool success = true;
    
    CommandLineArg current_arg = THIS_FILENAME; // NOTE: This program's name is always first!
    
    bool in_quotes = false;
    bool at_option = false;
    bool at_arg = false;
    
    while (*str != 0)
    {
        if (*str == '"')
        {
            in_quotes = !in_quotes;
            *str = 0;
        }
        else if (*str == ' ' && !in_quotes)
        {
            at_arg = false;
            *str = 0;
        }
        else if (*str == '/' && !at_arg && !in_quotes)
        {
            at_option = true;
        }
        else if (at_option)
        {
            at_option = false;
            
            if (*str == 'c')
            {
                current_arg = COPYRIGHT;
            }
            else if (*str == 'i')
            {
                current_arg = ICON_FILENAME;
            }
            else
            {
                char text_buffer[256];
                win32_printf(text_buffer, "Unrecognized option: %\n", str);
                success = false;
                break;
            }
        }
        else if (!at_arg)
        {
            at_arg = true;
            
            if (args[current_arg])
            {
                win32_print("Multiple values provided for the same argument.\n");
                success = false;
                break;
            }
            
            args[current_arg] = str;
            current_arg = EXE_FILENAME; // NOTE: No option is required for this argument.
        }
        
        ++str;
    }
    
    if (!args[EXE_FILENAME])
    {
        win32_print("No target executable file provided.\n");
        success = false;
    }
    
    return success;
}

void mainCRTStartup(void)
{
    int exit_code = 0;
    
    char *args[ARG_COUNT] = {0};
    if (parse_command_line(args, GetCommandLineA()))
    {
        void *exe_handle = BeginUpdateResourceA(args[EXE_FILENAME], true);
        if (exe_handle)
        {
            if (args[ICON_FILENAME])
            {
                update_icon(exe_handle, args[ICON_FILENAME]);
            }
            
            // NOTE: Set the string properties.
            
            size bytes = 1024*1024; // NOTE: 1MB.
            u8 *buf = (u8 *)VirtualAlloc(0, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            VS_VERSIONINFO *ver_info = (VS_VERSIONINFO *)buf;
            ver_info->length = sizeof(VS_VERSIONINFO) + sizeof(StringFileInfo) + sizeof(VarFileInfo); // NOTE: This is not the full size of the resource block yet, but it will be incremented as we procedurally add pieces below.
            ver_info->value_length = sizeof(VS_FIXEDFILEINFO);
            str_cpy_char_to_wide(ver_info->key, "VS_VERSION_INFO");
            
            ver_info->value.signature = 0xFEEF04BD; // NOTE: This is crucial. If you don't set it, you'll get no results.
            
            buf += sizeof(VS_VERSIONINFO);
            
            StringFileInfo *str_info = (StringFileInfo *)buf;
            str_info->length = sizeof(StringFileInfo);
            str_info->type = 1;
            str_cpy_char_to_wide(str_info->key, "StringFileInfo");
            
            buf += sizeof(StringFileInfo);
            
            // TODO: Like where we add a Var below, we could do a loop here to add multiple StringTables, one for each language. For now, the language is hard-coded to American English.
            StringTable *str_table = (StringTable *)buf;
            str_table->length = sizeof(StringTable);
            str_table->type = 1;
            str_cpy_char_to_wide(str_table->key, "040904b0");
            
            ver_info->length += sizeof(StringTable);
            str_info->length += sizeof(StringTable);
            buf += sizeof(StringTable);
            
            u16 total_str_size = 0;
            if (args[DESCRIPTION])
            {
                u16 str_size = add_string_resource((u16 *)buf, "FileDescription", args[DESCRIPTION]);
                buf += str_size;
                total_str_size += str_size;
            }
            if (args[COPYRIGHT])
            {
                u16 str_size = add_string_resource((u16 *)buf, "LegalCopyright", args[COPYRIGHT]);
                buf += str_size;
                total_str_size += str_size;
            }
            if (args[PRODUCT_NAME])
            {
                u16 str_size = add_string_resource((u16 *)buf, "ProductName", args[PRODUCT_NAME]);
                buf += str_size;
                total_str_size += str_size;
            }
            if (args[VERSION])
            {
                //ver_info->value.file_version_ms = 1;
                //ver_info->value.prod_version_ms = 1;
                
                u16 str_size = add_string_resource((u16 *)buf, "FileVersion", args[VERSION]);
                buf += str_size;
                total_str_size += str_size;
                
                str_size = add_string_resource((u16 *)buf, "ProductVersion", args[VERSION]);
                buf += str_size;
                total_str_size += str_size;
            }
            
            ver_info->length  += total_str_size;
            str_info->length  += total_str_size;
            str_table->length += total_str_size;
            
            VarFileInfo *var_info = (VarFileInfo *)buf;
            var_info->length = sizeof(VarFileInfo);
            str_cpy_char_to_wide(var_info->key, "VarFileInfo");
            
            buf += sizeof(VarFileInfo);
            
            // TODO: This part could become a loop to add multiple languages.
            Var *var = (Var *)buf;
            var->length = sizeof(Var);
            var->value_length = sizeof(u32);
            str_cpy_char_to_wide(var->key, "Translation");
            var->language = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);
            var->code_page = 0x04b0;
            
            ver_info->length += sizeof(Var);
            var_info->length += sizeof(Var);
            buf += sizeof(Var);
            
            bool result = UpdateResourceA(exe_handle, RT_VERSION, (char *)VS_VERSION_INFO, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), ver_info, ver_info->length);
            
            EndUpdateResourceA(exe_handle, false);
        }
        else
        {
            char buf[256];
            win32_printf(buf, "Not a valid executable file: %\n", args[EXE_FILENAME]);
            exit_code = 1;
        }
    }
    else
    {
        exit_code = 1;
    }
    
    ExitProcess(exit_code);
}
