#include "language.h"

#ifndef HEADER_CODER
#define HEADER_CODER

/**
 * @brief code line to binary dest
 * @return size of written data
 */
size_t coder_code(void *dest, Line *line);

/**
 * @brief decode coded by 'code' binary source to result
 * @return size of read data
 */
size_t coder_decode(void *source, Line *result);

/**
 * @brief update jmp and call addresses
 * @param position ptr there jump was coded
 * @param offset value which should be written
 */
void coder_modify_jump(void *position, Word offset);

#endif
