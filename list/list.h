#ifndef HEADER_LIST
#define HEADER_LIST

#include <stdlib.h>

typedef size_t Index;
#define NONE ((Index)-1) // NULL

/**
 * @brief List container based on index addressation
 */
typedef struct
{
	void *array;		 // defines all List memory
	size_t capacity;	 // in bytes
	size_t element_size; // in bytes, doesn't include inner info
	Index front;		 // first element
	Index back;			 // last element
	Index _free;		 // inner
	int _is_index;		 // inner
} List;
// for (size_t iter = list->front; iter != NONE; iter = list_right(list, iter))

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
 * @brief git pointer to right from 'element' element
 */
Index list_right(List *list, Index element);

/**
 * @brief git pointer to left from 'element' element
 */
Index list_left(List *list, Index element);

/**
 * @brief insert new element after 'element'
 * @return index of new element
 */
Index list_insert_after(List *list, Index element);

/**
 * @brief insert new element before 'element'
 * @return index of new element
 */
Index list_insert_before(List *list, Index element);

/**
 * @brief erase element from list
 */
void list_erase(List *list, Index element);

/**
 * @brief create first element
 * @return index of new element
 */
Index list_create_first(List *list);

/**
 * @brief add element to the end of list
 * @return index of new element
 */
Index list_push_back(List *list);

/**
 * @brief add element to the beginning of list
 * @return index of new element
 */
Index list_push_front(List *list);

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

Index list_at(List *list, Index position);

#endif