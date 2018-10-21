#define MAKELANGID(p, s)       ((((u16)(s)) << 10) | (u16)(p))
#define SUBLANG_DEFAULT        0x01
#define LANG_ENGLISH           0x09

void * __stdcall BeginUpdateResourceA(const char *filename, bool delete_existing);
bool   __stdcall UpdateResourceA     (void *handle, const char *type, const char *name, u16 language, void *data, u32 size);
bool   __stdcall EndUpdateResourceA  (void *handle, bool discard);

#define MAKEINTRESOURCE(a) (char *)((u64)a)

/* ICON Constants & Structs */

#define RT_ICON       ((char *)3)
#define RT_GROUP_ICON ((char *)14)
#define RT_VERSION    ((char *)16)

#pragma pack(push, 2)
// NOTE: Header that precedes icon structures, both in .ico files and in executables.
typedef struct {
    u16 reserved;       // NOTE: Always 0.
    u16 resource_type;  // NOTE: Always 1.
    u16 image_count;    // NOTE: The number of icons following this header.
} ICONDIR;

// NOTE: This data structure represents the format of an icon resource when it's stored in an .ico file. 
typedef struct {
    u8 width;           // NOTE: Width in pixels.
    u8 height;          // NOTE: Height in pixels.
    u8 color_count;     // NOTE: Colors in palette (0 if >= 8bpp).
    u8 reserved;        // NOTE: Always 0.
    u16 color_planes;   // NOTE: Color plane count. Should be 0 or 1.
    u16 bits_per_pixel; // NOTE: Bit-depth.
    u32 size;           // NOTE: Size of the image data in bytes.
    u32 image_offset;
} ICONDIRENTRY;

// NOTE: This data structure represents the format of an icon resource when it's stored in the resource section of an executable. Note that it differs crucially from ICONDIRENTRY in that its final member is a 2-byte ID rather than a 4-byte offset.
typedef struct {
    u8  width;
    u8  height;
    u8  color_count;
    u8  reserved;
    u16 color_planes;
    u16 bits_per_pixel;
    u32 size;
    u16 id;             // NOTE: Unique ID matching the one passed to UpdateResourceA along with the corresponding image data.
} GRPICONDIRENTRY;
#pragma pack(pop)

/* VERSION INFO Constants & Structs */

#define VS_VERSION_INFO 1

// NOTE: These structs used to format our block of binary data correctly before handing it to UpdateResourceA(). After every "key" member in the following structs, we need to be aligned to a 32-bit boundary, hence the occasional padding.

#pragma pack(push, 1)
typedef struct {
    u16 length;       // NOTE: Size of the whole var block in BYTES.
    u16 value_length; // NOTE: Always 0.
    u16 type;         // NOTE: Always 0 (binary).
    u16 key[12];      // NOTE: Always "VarFileInfo", null-terminated.
    u16 padding[1];   // NOTE: Align struct to a 32-bit boundary.
} VarFileInfo;

typedef struct {
    u16 length;       // NOTE: Size of this struct in BYTES.
    u16 value_length; // NOTE: Always 4 (size of 32-bit payload).
    u16 type;         // NOTE: Always 0 (binary).
    u16 key[12];      // NOTE: Always "Translation", null-terminated.
    u16 padding[1];   // NOTE: Align payload to a 32-bit boundary.
    u16 language;     // NOTE: Language ID.
    u16 code_page;    // TODO: What even is this?
} Var;

typedef struct {
    u16 length;       // NOTE: Size of the whole string block in BYTES.
    u16 value_length; // NOTE: Always 0.
    u16 type;         // NOTE: Always 1 (text).
    u16 key[15];      // NOTE: Always "StringFileInfo", null-terminated.
} StringFileInfo;

typedef struct {
    u16 length;       // NOTE: Size of table and its children strings in BYTES.
    u16 value_length; // NOTE: Always 0.
    u16 type;         // NOTE: Always 1 (text).
    u16 key[9]; // NOTE: 8-digit hex number as null-terminated utf16 string. First four characters are language ID, and seconds four are code page. 
} StringTable;

// NOTE: Shouldn't be used as a valid C structure because it has a bunch of variable length members. It's purely here for documentation. Instead, we construct this string data block manually.
typedef struct {
    u16 length;       // NOTE: Size of this struct in BYTES.
    u16 value_length; // NOTE: Size of "value" in WORDS (u16).
    u16 type;         // NOTE: Always 1 (text).
    u16 key[1];       // https://docs.microsoft.com/en-us/windows/desktop/menurc/string-str
    u16 padding[1];   // Align
    u16 value[1];
    // TODO: Do we potentially need padding here, to play nice with subsequent data blocks?
} String;

typedef struct {
    u32 signature;       // NOTE: Always 0xFEEF04BD.
    u32 struct_version;     
    u32 file_version_ms;
    u32 file_version_ls;
    u32 prod_version_ms;
    u32 prod_version_ls;
    u32 file_flags_mask;
    u32 file_flags;
    u32 file_os;
    u32 file_type;
    u32 file_subtype;
    u32 file_date_ms;
    u32 file_date_ls;
} VS_FIXEDFILEINFO;

typedef struct {
    u16 length;             // NOTE: Size of everything in BYTES.
    u16 value_length;       // NOTE: Size of "value" in BYTES.
    u16 type;               // NOTE: Type of data. 0 for binary, 1 for text.
    u16 key[16];            // NOTE: Always "VS_VERSION_INFO", null-terminated.
    u16 padding_1[1];       // NOTE: Align "value" on a 32-bit boundary.
    VS_FIXEDFILEINFO value;
    //u16 padding_2;          // TODO: Not sure if this is necessary.
} VS_VERSIONINFO;
#pragma pack(pop)
