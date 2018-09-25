#define MEM_RELEASE          0x8000
#define MEM_COMMIT           0x00001000
#define MEM_RESERVE          0x00002000
#define PAGE_READWRITE       0x04

void* __stdcall VirtualAlloc(void*, ptr, u32, u32);
sb32  __stdcall VirtualFree (void*, ptr, u32);
