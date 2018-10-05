#include "basic.h"
#include "string.c"

//#include "win32_crt.c"
#include "win32_memory.c"
#include "win32_io.c"

#include "resource.h"

#define MAKELANGID(p, s)       ((((u16)(s)) << 10) | (u16)(p))
#define SUBLANG_DEFAULT        0x01
#define LANG_ENGLISH           0x09

char * __stdcall GetCommandLineA     (void);
void * __stdcall BeginUpdateResourceA(const char*file_name, sb32 delete_existing_resources);
sb32   __stdcall UpdateResourceA     (void *handle, const char *type, const char *name, u16 language, void *data, u32 size);
sb32   __stdcall EndUpdateResourceA  (void *handle, sb32 discard);

typedef struct {
    b32 error;
    
    char *exe_filename;
    char *icon_filename;
    
    char *file_description;
    char *product_name;
    char *version;
    char *copyright;
} args_t;

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
    if (**full_string == '"') {
        ++*full_string;
        *filename = *full_string;
        while (**full_string != '"') {
            ++*full_string;
        }
    }
    else {
        *filename = *full_string;
        while (**full_string != ' ') {
            if (!**full_string) {
                return;
            }
            
            ++*full_string;
        }
    }
    
    **full_string = 0;
    ++*full_string;
}

static args_t parse_command_line_args(char *string) {
    // NOTE: Rather than allocate new space for the strings and copy them over, we just point our pointers to the starts of the args and replace delimiting spaces with null terminators.
    
    args_t result = {0};
    result.error = FALSE;
    
    // NOTE: Advance to the beginning of the arguments.
    while(*string != ' ') {
        if (!*string) {
            break;
        }
        if (*string == '"') {
            ++string;
            while (*string != '"') {
                ++string;
            }
        }
        ++string;
    }
    
    // NOTE: Consume an argument each time through this loop.
    while(*string) {
        if (*string == ' ') {
            ++string;
        }
        else if (*string == '/') {
            if (starts_with_substring(string, "/i:")) {
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
                result.error = TRUE;
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
                result.error = TRUE;
                break;
            }
        }
    }
    
    return result;
}

void main(void) {
    args_t args = parse_command_line_args(GetCommandLineA());
    
    if (!args.error) {
        void *exe_handle = BeginUpdateResourceA(args.exe_filename, TRUE);
        if (exe_handle) {
            if (args.icon_filename) {
                win32_file_contents icon = win32_read_file(args.icon_filename);
                if (!icon.error) {
                    icon_dir_header *id_header = (icon_dir_header *)icon.memory;
                    int image_count = id_header->image_count;
                    
                    //
                    icon_header new = {0};
                    new.resource_type = id_header->resource_type;
                    //
                    
                    icon_dir_entry *first_entry = (icon_dir_entry *)((u8 *)icon.memory + sizeof(icon_dir_header));
                    
                    for (u16 i = 0; i < image_count; ++i) {
                        icon_dir_entry *id_entry = first_entry + i;
                        
                        u32 size = id_entry->size;
                        u8 *memory = (u8 *)icon.memory + id_entry->offset;
                        icon_image *image = (icon_image *)memory;
                        if (image->header.biWidth <= 256)
                        {
                            sb32 result = UpdateResourceA(exe_handle, RT_ICON, (char *)((u64)i+102), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (void *)memory, size);
                            
                            new.data[new.image_count].width = id_entry->width;
                            new.data[new.image_count].height = id_entry->height;
                            new.data[new.image_count].color_count = id_entry->color_count;
                            new.data[new.image_count].reserved = id_entry->reserved;
                            new.data[new.image_count].color_planes = id_entry->color_planes;
                            new.data[new.image_count].bits_per_pixel = id_entry->bits_per_pixel;
                            new.data[new.image_count].size = id_entry->size;
                            new.data[new.image_count].id = i+102;
                            
                            new.image_count += 1;
                        }
                    }
                    
                    //u32 size = sizeof(icon_dir_header) + (sizeof(icon_dir_entry) * image_count);
                    u32 size = sizeof(icon_dir_header) + (sizeof(icon_data) * new.image_count);
                    
                    //sb32 result = UpdateResourceA(exe_handle, RT_GROUP_ICON, "MAINICON", MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), &new, size);
                    sb32 result = UpdateResourceA(exe_handle, RT_GROUP_ICON, "a", MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), &new, size);
                }
                else {
                    win32_print("Unable to find icon file.\n");
                }
            }
            
            ptr size = 1024*1024; // NOTE: 1MB.
            u8 *buf = (u8 *)VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
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
            
            // TODO: Like where we add a Var below, we could do a loop here to add multiple StringTables, one for each language.
            StringTable *str_table = (StringTable *)buf;
            str_table->length = sizeof(StringTable);
            str_table->type = 1;
            str_cpy_utf16(str_table->key, "040904b0");
            
            ver_info->length += sizeof(StringTable);
            str_info->length += sizeof(StringTable);
            buf += sizeof(StringTable);
            
            u16 total_str_size = 0;
            if (args.file_description) {
                u16 str_size = add_string(buf, "FileDescription", args.file_description);
                buf += str_size;
                total_str_size += str_size;
            }
            if (args.copyright) {
                u16 str_size = add_string(buf, "LegalCopyright", args.copyright);
                buf += str_size;
                total_str_size += str_size;
            }
            if (args.product_name) {
                u16 str_size = add_string(buf, "ProductName", args.product_name);
                buf += str_size;
                total_str_size += str_size;
            }
            if (args.version) {
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
            
            sb32 result = UpdateResourceA(exe_handle, RT_VERSION, (char *)VS_VERSION_INFO, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), ver_info, ver_info->length);
            
            EndUpdateResourceA(exe_handle, FALSE);
        }
        else {
            win32_print("Not a valid executable file.\n");
        }
    }
}
