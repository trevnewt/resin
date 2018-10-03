static int strlen(char *string)
{
    int result = 0;
    while(*string)
    {
        ++string;
        ++result;
    }
    
    return result;
}

static void strcpy(char *dst, char *src) {
    while(*src) {
        *dst++ = *src++;
    }
}

static void strcpy_utf16(u16 *dst, char *src) {
    while(*src) {
        *dst++ = *src++;
    }
}

static b32 starts_with_substring(char *string, char *substring) {
    while(*substring)
    {
        if (*substring != *string)
        {
            return FALSE;
        }
        substring++;
        string++;
    }
    
    return TRUE;
}

static b32 starts_with_substring_utf16(u16 *string, u16 *substring)
{
    while(*substring)
    {
        if (*substring != *string)
        {
            return FALSE;
        }
        substring++;
        string++;
    }
    
    return TRUE;
}
