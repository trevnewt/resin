/* WIN32 Constants, Structs, & Function Prototypes */

#define GENERIC_READ         0x80000000L
#define GENERIC_WRITE        0x40000000L

#define FILE_SHARE_READ      0x00000001
#define OPEN_EXISTING        3

#define INVALID_HANDLE_VALUE (void *)((size)-1)

#define STD_OUTPUT_HANDLE    (u32)-11

typedef struct {
    u32  nLength;
    void *lpSecurityDescriptor;
    bool bInheritHandle;
} SECURITY_ATTRIBUTES;

typedef union {
    struct {
        u32 LowPart;
        s32 HighPart;
    };
    struct {
        u32 LowPart;
        s32 HighPart;
    } u;
    s64 QuadPart;
} LARGE_INTEGER;

typedef struct {
    size Internal;
    size InternalHigh;
    union {
        struct {
            u32 Offset;
            u32 OffsetHigh;
        } DUMMYSTRUCTNAME;
        void *Pointer;
    } DUMMYUNIONNAME;
    void *hEvent;
} OVERLAPPED;

void * __stdcall CreateFileA  (const char *file_name, u32 desired_access, u32 share_mode, SECURITY_ATTRIBUTES *sec_attribs, u32, u32, void *);
bool   __stdcall GetFileSizeEx(void *, LARGE_INTEGER*);
bool   __stdcall ReadFile     (void *file_handle, void *buffer, u32 bytes_to_read, u32 *bytes_read, OVERLAPPED *overlapped);
void * __stdcall GetStdHandle (u32 handle);
bool   __stdcall WriteFile    (void *handle, const void *buffer, u32 bytes_to_write, u32 *bytes_written, OVERLAPPED *overlapped);
bool   __stdcall CloseHandle  (void *handle);

/* End of WIN32 stuff! */

typedef struct {
    void *memory;
    u32 size;
    bool error;
} Win32FileContents;

static void win32_print(char *str)
{
    u32 written = 0;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), str, (u32)str_len(str), &written, 0);
}

static void win32_printf(char *buf, char *format, char *insert)
{
    u32 len = (u32)str_len(format) + (u32)str_len(insert);
    char *dst = buf;
    
    while (*format)
    {
        if (*format == '%')
        {
            *format++;
            while (*insert)
            {
                *dst++ = *insert++;
            }
        }
        else
        {
            *dst++ = *format++;
        }
    }
    
    u32 written = 0;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, len, &written, 0);
}

static Win32FileContents win32_read_file(char *file_name)
{
    Win32FileContents result = {0};
    
    void *file_handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER file_size;
        GetFileSizeEx(file_handle, &file_size);
        u32 file_size_32 = (u32)file_size.QuadPart;
        result.memory = VirtualAlloc(0, file_size.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (result.memory)
        {
            u32 bytes_read;
            if (ReadFile(file_handle, result.memory, file_size_32, &bytes_read, 0) && file_size_32 == bytes_read)
            {
                result.size = file_size_32;
            }
            else
            {
                VirtualFree(result.memory, 0, MEM_RELEASE);
                result.memory = 0;
            }
        }
        else
        {
            result.error = true;
            // NOTE: LOGGING.
        }
        CloseHandle(file_handle);
    }
    else
    {
        result.error = true;
    }
    
    return result;
}
