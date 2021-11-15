#include <stdint.h>

#ifndef HEADER_LIST
#define HEADER_LIST

struct List_iterator;

typedef struct List_iterator
{
	struct List_iterator *left;
	struct List_iterator *right;
	uint64_t data_1; // it should be enough for most cases
	uint64_t data_2; // use only 64 bit - overhead costs are off scale
					 // data should be movable by copying
} List_iterator;

/**
 * @brief allocate memory for node
 */
List_iterator *list_create(uint64_t data_1, uint64_t data_2);

/**
 * @brief free memory at iter
 * @exception calls erase
 */
void list_remove(List_iterator *iter);

/**
 * @brief unlink iter with neighbors
 */
void list_erase(List_iterator *iter);

/**
 * @brief insert node in list
 * 
 * @param left node from list, after those should insert
 * @param right node which to insert (should be lone)
 */
void list_insert_righter(List_iterator *left, List_iterator *right);

#endif