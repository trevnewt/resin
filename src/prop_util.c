#include "basic.h"
#include "string.c"

#include "win32_memory.c"
#include "win32_io.c"

#define MAKEINTRESOURCEA(i) ((char *)((ptr)((u16)(i))))
#define MAKEINTRESOURCE     MAKEINTRESOURCEA

#define RT_ICON       MAKEINTRESOURCE(3)
#define RT_GROUP_ICON MAKEINTRESOURCE((ptr)(RT_ICON) + 11)

#define MAKELANGID(p, s)       ((((u16)(s)) << 10) | (u16)(p))
#define SUBLANG_DEFAULT        0x01
#define LANG_ENGLISH           0x09

void * __stdcall BeginUpdateResourceA(const char*file_name, sb32 delete_existing_resources);
sb32   __stdcall UpdateResourceA     (void *handle, const char *type, const char *name, u16 language, void *data, u32 size);
sb32   __stdcall EndUpdateResourceA  (void *handle, sb32 discard);
char * __stdcall GetCommandLineA     (void);

#pragma pack(push, 2)
typedef struct {
    u16 reserved_0;     // Must be 0.
    u16 resource_type;  // 1 for icons.
    u16 image_count;    // Number of images. In our case, must be 1.
    u8  width;          // Pixel width of the image.
    u8  height;         // Pixel height of the image.
    u8  color_count;    // Number of colors in image (0 if >=8bpp).
    u8  reserved_1;     // Must be 0.
    u16 color_planes;   // Color planes. ???
    u16 bits_per_pixel; // Must be 32.
    u32 size;           // Size in bytes of the image.
    u16 id;             // The ID of this image. Always 1?
} icon_data;
#pragma pack(pop)

// TODO: Formalize the lengths of these strings.
typedef struct {
    char exe_filename[256];
    char icon_filename[256];
    char product_name[256];
    char product_version[256];
    char copyright[256];
} args_t;

static args_t parse_command_line_args(char *string) {
    args_t result = {0};
    
    while(*string != ' ') {
        ++string;
    }
    
    while(string) {
        // TODO: Tab delimited?
        if (*string == ' ') {
            ++string;
        }
        else if (*string == '/') {
            if (string) {
            }
        }
        else {
            string_copy(result.exe_filename, string);
        }
    }
    
    return result;
}

void main(void) {
    args_t args = parse_command_line_args(GetCommandLineA());
    
    void *exe_handle = BeginUpdateResourceA(args.exe_filename, TRUE);
    if (exe_handle) {
        win32_file_contents icon = win32_read_file(args.icon_filename);
        
        // NOTE(trevor): Update the icon itself.
        
        UpdateResourceA(exe_handle, RT_ICON, MAKEINTRESOURCE(1), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), icon.memory, icon.size);
        
        // NOTE(trevor): Update the icon directory header.
        
        icon_data id;
        id.reserved_0     = 0;
        id.resource_type  = 1;
        id.image_count    = 1;
        id.width          = 32;
        id.height         = 32;
        id.reserved_1     = 0;
        id.color_planes   = 0;
        id.bits_per_pixel = 32;
        id.size           = icon.size;
        id.id             = 1;
        
        UpdateResourceA(exe_handle, (const char *)RT_GROUP_ICON, "MAINICON", MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), &id, sizeof(icon_data));
        
        EndUpdateResourceA(exe_handle, FALSE);
        
    }
    else {
        win32_print("Not a valid executable file.\n");
    }
}
