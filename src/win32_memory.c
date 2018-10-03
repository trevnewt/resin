#define MEM_RELEASE          0x8000
#define MEM_COMMIT           0x00001000
#define MEM_RESERVE          0x00002000
#define PAGE_READWRITE       0x04

void * __stdcall VirtualAlloc(void *starting_address, ptr size, u32 alloc_type, u32 protect_flags);
sb32   __stdcall VirtualFree (void *memory, ptr, u32);
