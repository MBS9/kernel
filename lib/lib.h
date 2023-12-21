#pragma once
#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

void init_mem(void* base);
void* malloc(size_t nitems, size_t size);
void* calloc(size_t nitems, size_t size);
void free(void *ptr);

void revstr(char *str1, int len);
char* itoa(int num, char* str, int base);
int strlen(char *n, char b);