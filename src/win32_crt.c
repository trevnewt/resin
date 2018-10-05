// NOTE: Silly MSVC generates a call to memset when you allocate 

#pragma function(memset)
void *memset(void *dst, int c, size_t count)
{
    u8 *bytes = (u8 *)dst;
    while (count--)
    {
        *bytes++ = (u8)c;
    }
    return dst;
}
