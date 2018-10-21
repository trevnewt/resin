static size str_len(char *str)
{
    size len = 0;
    while (str[len])
    {
        ++len;
    }
    
    return len;
}

static void str_cpy(char *dst, char *src)
{
    while (*src)
    {
        *dst++ = *src++;
    }
}

static void strcpy_char_to_wide(u16 *dst, char *src)
{
    while (*src)
    {
        *dst++ = *src++;
    }
}

static bool starts_with_substring(char *test, char *substring)
{
    while (*substring)
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
