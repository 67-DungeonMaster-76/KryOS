/*
 * string.c - String handling functions implementation
 * version 0.0.1
 */

#include "string.h"

/*
 * Copy memory area
 */
void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    
    while (n--) {
        *d++ = *s++;
    }
    
    return dest;
}

/*
 * Copy memory area (handles overlapping)
 */
void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    
    return dest;
}

/*
 * Fill memory with a constant byte
 */
void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
    
    while (n--) {
        *p++ = (uint8_t)c;
    }
    
    return s;
}

/*
 * Compare two memory areas
 */
int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    
    return 0;
}

/*
 * Copy string
 */
char *strcpy(char *dest, const char *src) {
    char *d = dest;
    
    while ((*d++ = *src++) != '\0')
        ;
    
    return dest;
}

/*
 * Copy string with length limit
 */
char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    
    while (n && (*d++ = *src++) != '\0') {
        n--;
    }
    
    while (n--) {
        *d++ = '\0';
    }
    
    return dest;
}

/*
 * String concatenation
 */
char *strcat(char *dest, const char *src) {
    char *d = dest;
    
    while (*d) {
        d++;
    }
    
    while ((*d++ = *src++) != '\0')
        ;
    
    return dest;
}

/*
 * String concatenation with length limit
 */
char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    
    while (*d) {
        d++;
    }
    
    while (n-- && (*d = *src) != '\0') {
        d++;
        src++;
    }
    
    *d = '\0';
    
    return dest;
}

/*
 * String comparison
 */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    
    return *(const uint8_t *)s1 - *(const uint8_t *)s2;
}

/*
 * String comparison with length limit
 */
int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
        n--;
    }
    
    if (n == 0) {
        return 0;
    }
    
    return *(const uint8_t *)s1 - *(const uint8_t *)s2;
}

/*
 * Compare strings ignoring case
 */
int strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        char c1 = *s1;
        char c2 = *s2;
        
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        
        if (c1 != c2) {
            return c1 - c2;
        }
        
        s1++;
        s2++;
    }
    
    return *s1 - *s2;
}

/*
 * Compare strings ignoring case with length limit
 */
int strncasecmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && *s2) {
        char c1 = *s1;
        char c2 = *s2;
        
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        
        if (c1 != c2) {
            return c1 - c2;
        }
        
        s1++;
        s2++;
        n--;
    }
    
    if (n == 0) {
        return 0;
    }
    
    return *s1 - *s2;
}

/*
 * Find character in string
 */
char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }
    
    if ((char)c == '\0') {
        return (char *)s;
    }
    
    return (char *)0;
}

/*
 * Find last occurrence of character in string
 */
char *strrchr(const char *s, int c) {
    const char *last = (char *)0;
    
    while (*s) {
        if (*s == (char)c) {
            last = s;
        }
        s++;
    }
    
    if ((char)c == '\0') {
        return (char *)s;
    }
    
    return (char *)last;
}

/*
 * Find first occurrence of any character from accept
 */
char *strpbrk(const char *s, const char *accept) {
    while (*s) {
        const char *a = accept;
        while (*a) {
            if (*s == *a) {
                return (char *)s;
            }
            a++;
        }
        s++;
    }
    
    return (char *)0;
}

/*
 * Find first occurrence of substring
 */
char *strstr(const char *haystack, const char *needle) {
    if (*needle == '\0') {
        return (char *)haystack;
    }
    
    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;
        
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        
        if (*n == '\0') {
            return (char *)haystack;
        }
        
        haystack++;
    }
    
    return (char *)0;
}

/*
 * Get string length
 */
size_t strlen(const char *s) {
    size_t len = 0;
    
    while (*s++) {
        len++;
    }
    
    return len;
}

/*
 * Get string length with limit
 */
size_t strnlen(const char *s, size_t maxlen) {
    size_t len = 0;
    
    while (len < maxlen && *s++) {
        len++;
    }
    
    return len;
}

/*
 * Get length of prefix consisting only of accept characters
 */
size_t strspn(const char *s, const char *accept) {
    size_t len = 0;
    
    while (*s) {
        const char *a = accept;
        while (*a && *a != *s) {
            a++;
        }
        
        if (*a == '\0') {
            break;
        }
        
        len++;
        s++;
    }
    
    return len;
}

/*
 * Get length of prefix consisting of characters not in reject
 */
size_t strcspn(const char *s, const char *reject) {
    size_t len = 0;
    
    while (*s) {
        const char *r = reject;
        while (*r && *r != *s) {
            r++;
        }
        
        if (*r) {
            break;
        }
        
        len++;
        s++;
    }
    
    return len;
}

/* Static buffer for strtok */
static char *strtok_next = (char *)0;

/*
 * Tokenize string
 */
char *strtok(char *str, const char *delim) {
    char *token;
    
    if (str == (char *)0) {
        str = strtok_next;
    }
    
    if (str == (char *)0) {
        return (char *)0;
    }
    
    /* Skip leading delimiters */
    str += strspn(str, delim);
    
    if (*str == '\0') {
        strtok_next = (char *)0;
        return (char *)0;
    }
    
    token = str;
    
    /* Find end of token */
    str += strcspn(str, delim);
    
    if (*str) {
        *str++ = '\0';
    }
    
    strtok_next = str;
    
    return token;
}

/*
 * Reverse string in place
 */
char *strrev(char *s) {
    char *start = s;
    char *end = s;
    char tmp;
    
    while (*end) {
        end++;
    }
    end--;
    
    while (start < end) {
        tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }
    
    return s;
}

/*
 * Convert string to uppercase
 */
char *strupr(char *s) {
    char *p = s;
    
    while (*p) {
        if (*p >= 'a' && *p <= 'z') {
            *p -= 32;
        }
        p++;
    }
    
    return s;
}

/*
 * Convert string to lowercase
 */
char *strlwr(char *s) {
    char *p = s;
    
    while (*p) {
        if (*p >= 'A' && *p <= 'Z') {
            *p += 32;
        }
        p++;
    }
    
    return s;
}