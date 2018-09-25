#define GENERIC_READ         0x80000000L
#define GENERIC_WRITE        0x40000000L
#define FILE_SHARE_READ      0x00000001
#define OPEN_EXISTING        3
#define CREATE_ALWAYS        2
#define INVALID_HANDLE_VALUE (void *)0xffffffffffffffff
#define STD_OUTPUT_HANDLE    (u32)-11

typedef struct {
    u32  nLength;
    void *lpSecurityDescriptor;
    sb32 bInheritHandle;
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
    ptr Internal;
    ptr InternalHigh;
    union {
        struct {
            u32 Offset;
            u32 OffsetHigh;
        } DUMMYSTRUCTNAME;
        void *Pointer;
    } DUMMYUNIONNAME;
    void *hEvent;
} OVERLAPPED;

void * __stdcall CreateFileA  (const char *file_name, u32 desired_access, u32 share_mode, SECURITY_ATTRIBUTES*, u32, u32, void *);
sb32   __stdcall GetFileSizeEx(void *, LARGE_INTEGER*);
sb32   __stdcall ReadFile     (void *file_handle, void *buffer, u32 bytes_to_read, u32 *bytes_read, OVERLAPPED *overlapped);
void * __stdcall GetStdHandle (u32 handle);
sb32   __stdcall WriteFile    (void *handle, const void *buffer, u32 bytes_to_write, u32 *bytes_written, OVERLAPPED *overlapped);
sb32   __stdcall CloseHandle  (void *handle);

static void win32_print(char *string) {
    void *console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    
    u32 *bytes_written = 0;
    WriteFile(console_handle, string, strlen(string), bytes_written, 0);
}

typedef struct {
    void *memory;
    u32 size;
} win32_file_contents;

static win32_file_contents win32_read_file(char *filename) {
    win32_file_contents result = {0};
    
    void *file_handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
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
            // LOGGING.
        }
        CloseHandle(file_handle);
    }
    else {
        win32_print("something!");
    }
    
    return result;
}
