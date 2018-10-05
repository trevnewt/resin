static u32 str_len(char *str)
{
    int result = 0;
    while(*str)
    {
        ++str;
        ++result;
    }
    
    return result;
}

static void str_cpy(char *dst, char *src)
{
    while(*src)
    {
        *dst++ = *src++;
    }
}

static void str_cpy_utf16(u16 *dst, char *src)
{
    while(*src)
    {
        *dst++ = *src++;
    }
}

static b32 starts_with_substring(char *test, char *substring)
{
    while(*substring)
    {
        if (*substring != *test)
        {
            return FALSE;
        }
        substring++;
        test++;
    }
    
    return TRUE;
}
