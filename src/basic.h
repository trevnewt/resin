//#ifdef MSVC
typedef __int8  s8;
typedef __int16 s16;
typedef __int32 s32;
typedef __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;
//#endif

typedef s32 sb32;

#ifdef X64
typedef u64 ptr;
#else
typedef u32 ptr;
#endif

#define FALSE 0
#define TRUE  1
