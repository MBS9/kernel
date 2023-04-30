#pragma once
#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

void init_mem(void* framebufferEnd);
void* malloc(size_t nitems, size_t size);
void* calloc(size_t nitems, size_t size);
