static u32 str_len(char *str)
{
    u32 result = 0;
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

static bool starts_with_substring(char *test, char *substring)
{
    while(*substring)
    {
        if (*substring != *test)
        {
            return false;
        }
        substring++;
        test++;
    }
    
    return true;
}
