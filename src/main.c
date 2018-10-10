#include "basic.h"
#include "string.c"

#include "win32_memory.c"
#include "win32_io.c"

#include "resource.h"

#define MAKELANGID(p, s)       ((((u16)(s)) << 10) | (u16)(p))
#define SUBLANG_DEFAULT        0x01
#define LANG_ENGLISH           0x09

void   __stdcall ExitProcess         (u32 exit_code);
char * __stdcall GetCommandLineA     (void);
void * __stdcall BeginUpdateResourceA(const char *file_name, bool delete_existing_resources);
bool __stdcall UpdateResourceA     (void *handle, const char *type, const char *name, u16 language, void *data, u32 size);
bool __stdcall EndUpdateResourceA  (void *handle, bool discard);

typedef struct {
    bool error;
    
    char *exe_filename;
    char *icon_filename;
    
    char *file_description;
    char *product_name;
    char *version;
    char *copyright;
} Command_Line_Args;

static u16 add_string(u8 *buf, char *key, char *val)
{
    u16 key_len = (u16)str_len(key) + 1; // NOTE: +1 for \0 terminator.
    u16 val_len = (u16)str_len(val) + 1; // NOTE: +1 for \0 terminator.
    u16 pad_1 = (key_len % 2) ? 0 : 1; // NOTE: The first pad comes after three 16-bit values and then the key. If the key has an even number of elements, then we need a 16-bit pad to align the the "value" array to a 32-bit boundary.
    u16 pad_2 = (val_len % 2) ? 1 : 0;// NOTE: This pad comes after the value array, which we just ensured will start at a 32-bit boundary. If the value has an odd number of elements, then we need a 16-bit pad to align the start of the next block to a 32-bit boundary.
    u16 total_size = sizeof(u16)*(3 + key_len + pad_1 + val_len + pad_2);
    
    u16 *word = (u16 *)buf;
    
    *word++ = total_size; // NOTE: Size of block in BYTES.
    *word++ = val_len;    // NOTE: Size of value in WORDS.
    *word++ = 1;          // NOTE: Type. 0 for binary, 1 for text.
    str_cpy_utf16(word, key);
    word += key_len;
    word += pad_1;
    str_cpy_utf16(word, val);
    word += val_len;
    
    return total_size;
}

static void consume_filename(char **full_string, char **filename) {
    if (**full_string == '"')
    {
        ++*full_string;
        *filename = *full_string;
        while (**full_string != '"')
        {
            ++*full_string;
        }
    }
    else
    {
        *filename = *full_string;
        while (**full_string != ' ')
        {
            if (!**full_string)
            {
                return;
            }
            
            ++*full_string;
        }
    }
    
    **full_string = 0;
    ++*full_string;
}

static Command_Line_Args parse_command_line_args(char *string)
{
    // NOTE: Rather than allocate new space for the strings and copy them over, we just set our pointers to the starts of the args and replace delimiting spaces with null terminators.
    
    Command_Line_Args result = {0};
    result.error = false;
    
    // NOTE: Advance to the beginning of the arguments.
    while(*string != ' ')
    {
        if (!*string)
        {
            break;
        }
        if (*string == '"')
        {
            ++string;
            while (*string != '"')
            {
                ++string;
            }
        }
        ++string;
    }
    
    // NOTE: Consume an argument each time through this loop.
    while(*string)
    {
        if (*string == ' ')
        {
            ++string;
        }
        else if (*string == '/')
        {
            if (starts_with_substring(string, "/i:"))
            {
                string += 3;
                consume_filename(&string, &result.icon_filename);
            }
            else if (starts_with_substring(string, "/c:")) {
                string += 3;
                consume_filename(&string, &result.copyright);
            }
            else if (starts_with_substring(string, "/n:")) {
                string += 3;
                consume_filename(&string, &result.product_name);
            }
            else if (starts_with_substring(string, "/d:")) {
                string += 3;
                consume_filename(&string, &result.file_description);
            }
            else if (starts_with_substring(string, "/v:")) {
                string += 3;
                consume_filename(&string, &result.version);
            }
            else {
                win32_print("Unrecognized option.\n");
                result.error = true;
                break;
            }
        }
        else {
            if (!result.exe_filename)
            {
                consume_filename(&string, &result.exe_filename);
            }
            else
            {
                win32_print("You can't specify two executables!\n");
                result.error = true;
                break;
            }
        }
    }
    
    return result;
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
        win32_print("Unable to find icon file.\n");
    }
}

void mainCRTStartup(void)
{
    Command_Line_Args args = parse_command_line_args(GetCommandLineA());
    
    if (!args.error)
    {
        void *exe_handle = BeginUpdateResourceA(args.exe_filename, true);
        if (exe_handle)
        {
            if (args.icon_filename)
            {
                update_icon(exe_handle, args.icon_filename);
            }
            
            // NOTE: Set the string properties.
            
            size bytes = 1024*1024; // NOTE: 1MB.
            u8 *buf = (u8 *)VirtualAlloc(0, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            VS_VERSIONINFO *ver_info = (VS_VERSIONINFO *)buf;
            ver_info->length = sizeof(VS_VERSIONINFO) + sizeof(StringFileInfo) + sizeof(VarFileInfo); // NOTE: This is not the full size of the resource block yet, but it will be incremented as we procedurally add pieces below.
            ver_info->value_length = sizeof(VS_FIXEDFILEINFO);
            str_cpy_utf16(ver_info->key, "VS_VERSION_INFO");
            
            ver_info->value.signature = 0xFEEF04BD; // NOTE: This is crucial. If you don't set it, you'll get no results.
            
            buf += sizeof(VS_VERSIONINFO);
            
            StringFileInfo *str_info = (StringFileInfo *)buf;
            str_info->length = sizeof(StringFileInfo);
            str_info->type = 1;
            str_cpy_utf16(str_info->key, "StringFileInfo");
            
            buf += sizeof(StringFileInfo);
            
            // TODO: Like where we add a Var below, we could do a loop here to add multiple StringTables, one for each language. For now, the language is hard-coded to American English.
            StringTable *str_table = (StringTable *)buf;
            str_table->length = sizeof(StringTable);
            str_table->type = 1;
            str_cpy_utf16(str_table->key, "040904b0");
            
            ver_info->length += sizeof(StringTable);
            str_info->length += sizeof(StringTable);
            buf += sizeof(StringTable);
            
            u16 total_str_size = 0;
            if (args.file_description)
            {
                u16 str_size = add_string(buf, "FileDescription", args.file_description);
                buf += str_size;
                total_str_size += str_size;
            }
            if (args.copyright)
            {
                u16 str_size = add_string(buf, "LegalCopyright", args.copyright);
                buf += str_size;
                total_str_size += str_size;
            }
            if (args.product_name)
            {
                u16 str_size = add_string(buf, "ProductName", args.product_name);
                buf += str_size;
                total_str_size += str_size;
            }
            if (args.version)
            {
                //ver_info->value.file_version_ms = 1;
                //ver_info->value.prod_version_ms = 1;
                
                u16 str_size = add_string(buf, "FileVersion", args.version);
                buf += str_size;
                total_str_size += str_size;
                
                str_size = add_string(buf, "ProductVersion", args.version);
                buf += str_size;
                total_str_size += str_size;
            }
            
            ver_info->length  += total_str_size;
            str_info->length  += total_str_size;
            str_table->length += total_str_size;
            
            VarFileInfo *var_info = (VarFileInfo *)buf;
            var_info->length = sizeof(VarFileInfo);
            str_cpy_utf16(var_info->key, "VarFileInfo");
            
            buf += sizeof(VarFileInfo);
            
            // TODO: This part could become a loop to add multiple languages.
            Var *var = (Var *)buf;
            var->length = sizeof(Var);
            var->value_length = sizeof(u32);
            str_cpy_utf16(var->key, "Translation");
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
            win32_print("Not a valid executable file.\n");
        }
    }
    
    ExitProcess(0);
}
