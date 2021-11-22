/**
 * @file map.h
 * @brief Map (unordered) implementation - associative container
 */

#include <stdlib.h>
#include <stdbool.h>

#ifndef HEADER_DICTIONARY
#define HEADER_DICTIONARY

typedef struct
{
	size_t key_size;						   // size for Key
	size_t value_size;						   // size for Val
	int (*comparator)(void *, void *);		   // function to compare Keys
	void (*move_key)(void *dest, void *src);   // function to move new Key from src to dest, NULL if Map should use given to insert pointer (just allocated data)
	void (*move_value)(void *dest, void *src); // function to move new Val from src to dest, NULL if Map should use given to insert pointer (just allocated data)
	void (*erase_key)(void *key);			   // function to erase Key from key, NULL if Map shouldn't release pointer (static variables)
	void (*erase_value)(void *value);		   // function to erase Val from val, NULL if Map shouldn't release pointer (static variables)
} Map_structure;

/**
 * @brief frees map and initialise Map on this place
 * 
 * @param map place to create Map
 * @param structure description of Map
 */
void map_initialise(void *map, Map_structure *structure);

/**
 * @brief inserts or replaces map[key]=value
 * 
 * @return was it before inserting (was it replaced)
 */
bool map_insert(void *map, void *key, void *value);

/**
 * @brief erases map[key] if it was
 * 
 * @return was it before erasing (was smth erased)
 */
bool map_erase(void *map, void *key);

/**
 * @brief get count of elements in map
 */
size_t map_size(void *map);

/**
 * @brief find map[key]
 * 
 * @return pointer to Value or NULL if it wasn't found
 */
void *map_find_value(void *map, void *key);

/**
 * @brief just destructor of map
 * 
 * @exception map will point to foreign memory
 */
void map_destruct(void *map);

/*
begin, end, iter_inc ?
*/

#endif