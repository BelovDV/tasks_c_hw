#include <stdlib.h>

#ifndef HEADER_ALLOCATOR
#define HEADER_ALLOCATOR

/**
 * @brief allocate { 8, 16, 32, 64, 128, 256 } bytes
 */
void *allocate(size_t size);

/**
 * @brief deallocate allocated ptr
 * @param size should be as same as was passed to allocate
 */
void deallocate(void *ptr, size_t size);

/**
 * @brief save graphviz diagram of memory pages
 */
void alloc_dump(const char *engine,
				const char *file_text, const char *file_image);

#endif