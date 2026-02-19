/*
 * string.h - String handling functions
 * version 0.0.1
 */

#ifndef STRING_H
#define STRING_H

#include "stdint.h"

/* Copying functions */

/* Copy memory area */
void *memcpy(void *dest, const void *src, size_t n);

/* Copy memory area (handles overlapping) */
void *memmove(void *dest, const void *src, size_t n);

/* Fill memory with a constant byte */
void *memset(void *s, int c, size_t n);

/* Compare two memory areas */
int memcmp(const void *s1, const void *s2, size_t n);

/* Copy string */
char *strcpy(char *dest, const char *src);

/* Copy string with length limit */
char *strncpy(char *dest, const char *src, size_t n);

/* String concatenation */
char *strcat(char *dest, const char *src);

/* String concatenation with length limit */
char *strncat(char *dest, const char *src, size_t n);

/* String comparison */
int strcmp(const char *s1, const char *s2);

/* String comparison with length limit */
int strncmp(const char *s1, const char *s2, size_t n);

/* Compare strings ignoring case */
int strcasecmp(const char *s1, const char *s2);

/* Compare strings ignoring case with length limit */
int strncasecmp(const char *s1, const char *s2, size_t n);

/* Search functions */

/* Find character in string */
char *strchr(const char *s, int c);

/* Find last occurrence of character in string */
char *strrchr(const char *s, int c);

/* Find first occurrence of any character from accept */
char *strpbrk(const char *s, const char *accept);

/* Find first occurrence of substring */
char *strstr(const char *haystack, const char *needle);

/* Length functions */

/* Get string length */
size_t strlen(const char *s);

/* Get string length with limit */
size_t strnlen(const char *s, size_t maxlen);

/* Span functions */

/* Get length of prefix consisting only of accept characters */
size_t strspn(const char *s, const char *accept);

/* Get length of prefix consisting of characters not in reject */
size_t strcspn(const char *s, const char *reject);

/* Tokenize string */
char *strtok(char *str, const char *delim);

/* Other functions */

/* Reverse string in place */
char *strrev(char *s);

/* Convert string to uppercase */
char *strupr(char *s);

/* Convert string to lowercase */
char *strlwr(char *s);

#endif /* STRING_H */
