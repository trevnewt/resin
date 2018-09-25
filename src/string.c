static int strlen(char *string)
{
    int result = 0;
    while(string)
    {
        ++string;
        ++result;
    }
    
    return result;
}

static void string_copy(char *dst, char *src)
{
    while(src)
    {
        *dst++ = *src++;
    }
}
