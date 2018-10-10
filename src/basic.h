#ifdef MSVC
typedef __int8  s8;
typedef __int16 s16;
typedef __int32 s32;
typedef __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;
#endif

typedef s32 bool;

#ifdef x86
typedef u32 size;
#else
typedef u64 size;
#endif

#define false 0
#define true  1
