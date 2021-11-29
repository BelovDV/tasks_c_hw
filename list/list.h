#ifndef HEADER_LIST
#define HEADER_LIST

#include <stdlib.h>

/**
 * @brief List container based on index addressation
 */
typedef struct
{
	void *array;		  // defines all List memory
	size_t capacity;	  // in bytes
	size_t element_size;  // in bytes, doesn't include inner info
	size_t free_position; // offset in bytes
} List;

typedef size_t Index;
#define NONE ((Index)-1) // NULL

/**
 * @brief get initialized container List
 * @return root element
 * @exception previous values of list will be negletted
 */
void list_initialize(List *list, size_t element_size);

/**
 * @brief remove all elements and free all memory
 * @exception destructors of elements will be negletted
 */
void list_destruct(List *list);

/**
 * @brief get new spare element from list
 */
Index list_new(List *list);

/**
 * @brief git pointer to right from 'element' element
 */
Index list_right(List *list, Index element);

/**
 * @brief git pointer to left from 'element' element
 */
Index list_left(List *list, Index element);

/**
 * @brief insert 'inserting' after 'guideline'
 */
void list_insert_after(List *list, Index guideline, Index inserting);

/**
 * @brief insert 'before' after 'guideline'
 */
void list_insert_before(List *list, Index guideline, Index inserting);

/**
 * @brief erase element from list
 */
void list_erase(List *list, Index element);

/**
 * @brief get value from index
 */
void *list_element(List *list, Index element);

/**
 * @brief print inner info to stream
 */
void list_dump(void *stream, List *list);

/**
 * @brief graphviz...
 * @param printer (stream, element) -> write label name
 */
void list_graph(void *stream, List *list, void (*printer)(void *, void *));

#endif