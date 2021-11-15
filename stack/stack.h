#include <stdlib.h>
#include <stdint.h>

#ifndef HEADER_STACK
#define HEADER_STACK

typedef struct
{
	uint64_t hash;
	size_t size;
	size_t capacity;
	size_t element_sz;
	void *array;
} Stack_structure;

/**
 * @brief initialise Stack_structure for using
 * @param stack array will be freed
 * @param element_sz size of future elements
 */
void stack_initialise(Stack_structure *stack, size_t element_sz);

/**
 * @brief free array, should be called at the end
 */
void stack_destruct(Stack_structure *stack);

/**
 * @brief get pointer to top of the stack
 */
void *stack_top(const Stack_structure *stack);

/**
 * @brief push to stack element from pointer
 */
void stack_push(Stack_structure *stack, void *element);

/**
 * @brief pop from stack
 */
void stack_pop(Stack_structure *stack);

/**
 * @brief print info of stack to stderr
 */
void stack_dump(const Stack_structure *stack);

enum
{
	stack_err_nothing,
	stack_err_null_initialise,
	stack_err_null_stack,
	stack_err_null_element,
	stack_err_null_array,
	stack_err_calloc_err,
	stack_err_empty_pop,
	stack_err_empty_top,
	stack_err_wrong_hash,
	stack_err_wrong_canary,
	stack_err_wrong_capacity,
};

// #define STACK_NOT_FALL // change errno, but not exit with error
// #define STACK_HASH_ALL // hash all values

#endif