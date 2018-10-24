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

static void str_cpy_char_to_wide(u16 *dst, char *src)
{
    while (*src)
    {
        *dst++ = *src++;
    }
}
