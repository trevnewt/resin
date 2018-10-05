/* Icon */

#define RT_ICON       ((char *)3)
#define RT_GROUP_ICON ((char *)14)
#define RT_VERSION    ((char *)16)

#pragma pack(push, 2)
typedef struct {
    u16 reserved;
    u16 resource_type;
    u16 image_count;
} icon_dir_header;

typedef struct {
    u8 width;
    u8 height;
    u8 color_count;
    u8 reserved;
    u16 color_planes;
    u16 bits_per_pixel;
    u32 size;
    u32 offset;
} icon_dir_entry;

typedef struct {
    u8  width;          // Pixel width of the image.
    u8  height;         // Pixel height of the image.
    u8  color_count;    // Number of colors in pallette (0 if >=8bpp).
    u8  reserved;       // Must be 0.
    u16 color_planes;   // Color planes. Should be 0 or 1.
    u16 bits_per_pixel; // Must be 32.
    u32 size;           // Size in bytes of the image.
    u16 id;             // The ID of this image. Always 1?
} icon_data;

typedef struct {
    u16 reserved;       // NOTE:
    u16 resource_type;  //
    u16 image_count;    //
    icon_data data[10]; //
} icon_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    u32 biSize;
    s32 biWidth;
    s32 biHeight;
    u16 biPlanes;
    u16 biBitCount;
    u32 biCompression;
    u32 biSizeImage;
    s32 biXPelsPerMeter;
    s32 biYPelsPerMeter;
    u32 biClrUsed;
    u32 biClrImportant;
} BITMAPINFOHEADER;

typedef struct {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD;

typedef struct {
    BITMAPINFOHEADER header;
    RGBQUAD quad[1];
    u8 a[1];
    u8 b[1];
} icon_image;
#pragma pack(pop)

/* Version Info */

#define VS_VERSION_INFO 1

// NOTE: Structs used to format our block of binary data correctly before handing it to UpdateResourceA(). After every "key" member in the following structs, we need to be aligned to a 32-bit boundary, hence the occasional padding.

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
