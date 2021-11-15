#include "stack.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#ifndef STACK_NOT_FALL
#include <assert.h>
#define check(condition, id) assert(condition)
#define check_bool(condition, id) assert(condition)
#else
#define check(condition, id) \
	if (!(condition))        \
	{                        \
		errno = id;          \
		return;              \
	}
#define check_bool(condition, id) \
	if (!(condition))             \
	{                             \
		errno = id;               \
		return false;             \
	}
#endif

#define stack_at(stack, n) \
	((void *)((uint8_t *)stack->array + (n)*stack->element_sz))
// stack[0] - canary, stack[capacity + 1] - canary

static const uint8_t canary = 123;

uint64_t hash(const void *data, size_t size)
{
	uint64_t result = 0;
	const uint8_t *iter = data;
	for (size_t i = 0; i < size; ++i)
	{
		result ^= 5 + result << 5;
		result ^= 9 + result >> 7;
		result += iter[i];
	}
	return result;
}

static uint64_t stack_hash(const Stack_structure *stack)
{
	uint64_t result = hash((uint64_t *)stack + 1, sizeof(*stack));
#ifdef STACK_HASH_ALL
	result <<= 32;
	result += (uint32_t)hash(stack->array,
							 (stack->capacity + 2) * stack->element_sz);
#endif
	return result;
}

static bool stack_check(const Stack_structure *stack)
{
	check_bool(stack->array != NULL, stack_err_null_array);
	check_bool(stack->size <= stack->capacity, stack_err_wrong_capacity);

	check_bool(*(uint8_t *)stack_at(stack, 0) == canary,
			   stack_err_wrong_canary);
	check_bool(*(uint8_t *)stack_at(stack, stack->capacity + 1) == canary,
			   stack_err_wrong_canary);

	check_bool(stack->hash == stack_hash(stack), stack_err_wrong_hash);
	return true;
}

void stack_initialise(Stack_structure *stack, size_t element_sz)
{
	check(stack != NULL, stack_err_null_stack);

	stack->capacity = 0;
	stack->size = 0;
	stack->element_sz = element_sz;

	free(stack->array);
	stack->array = calloc(2, element_sz);
	check(stack->array != NULL, stack_err_calloc_err);
	memset(stack->array, canary, element_sz * 2);

	stack->hash = stack_hash(stack);
}

void stack_destruct(Stack_structure *stack)
{
	check(stack != NULL, stack_err_null_stack);
	if (!stack_check(stack))
		return;

	free(stack->array);

	stack->array = NULL;
	stack->capacity = 0;
	stack->size = 0;
}

void *stack_top(const Stack_structure *stack)
{
	check_bool(stack != NULL, stack_err_null_stack);
	if (!stack_check(stack))
		return NULL;
	check_bool(stack->size > 0, stack_err_empty_top);

	return stack_at(stack, stack->size);
}

void stack_reserve(Stack_structure *stack, size_t new_cap) // nonstatic
{
	check(stack != NULL, stack_err_null_stack);
	if (!stack_check(stack))
		return;

	//if (stack->capacity == new_cap)
	//	return;

	void *new_array = calloc(new_cap + 2, stack->element_sz);
	check(new_array != NULL, stack_err_calloc_err);

	size_t old = stack->capacity + 1;
	if (new_cap < old)
		old = new_cap;
	memcpy(new_array, stack->array, old * stack->element_sz);
	free(stack->array);
	stack->array = new_array;
	stack->capacity = new_cap;
	memset(stack_at(stack, stack->capacity + 1), canary, stack->element_sz);

	stack->hash = stack_hash(stack);
}

void stack_push(Stack_structure *stack, void *element)
{
	check(stack != NULL, stack_err_null_stack);
	check(element != NULL, stack_err_null_element);
	if (!stack_check(stack))
		return;

	if (stack->size == stack->capacity)
		stack_reserve(stack, stack->capacity * 2 + 4);
	check(stack->size != stack->capacity, stack_err_calloc_err);

	stack->size += 1;
	memcpy(stack_at(stack, stack->size), element, stack->element_sz);

	stack->hash = stack_hash(stack);
}

void stack_pop(Stack_structure *stack)
{
	check(stack != NULL, stack_err_null_stack);
	if (!stack_check(stack))
		return;
	check(stack->size > 0, stack_err_empty_pop);

	while (stack->size < stack->capacity / 4)
		stack_reserve(stack, stack->capacity / 4 + 2);
	//check(stack->size >= stack->capacity / 4 + 2, stack_err_calloc_err);

	memset(stack_at(stack, stack->size), 0, stack->element_sz);
	stack->size -= 1;

	stack->hash = stack_hash(stack);
}

static void print_raw_hex(FILE *stream, void *data, size_t size)
{
	uint8_t *element = data;
	for (size_t byte = 0; byte < size; ++byte)
	{
		fprintf(stream, "%x ", *element);
		++element;
	}
}

void stack_dump(const Stack_structure *stack)
{
	check(stack != NULL, stack_err_null_stack);

	fprintf(stderr,
			"DUMP:\n"
			"\tsize     = %lu\n"
			"\tcapacity = %lu\n"
			"\thash     = %lx\n"
			"\tarray    = %p\n"
			"\tarray:\n",
			stack->size, stack->capacity, stack->hash, stack->array);

	for (size_t i = 1; i < stack->size + 1; ++i)
	{
		fprintf(stderr, "\t\tstack[%lu]: \t", i);
		print_raw_hex(stderr, stack_at(stack, i), stack->element_sz);
		fprintf(stderr, "\n");
	}
	for (size_t i = stack->size + 1; i < stack->capacity + 1; ++i)
	{
		fprintf(stderr, "\t\t-stack[%lu]:\t", i);
		print_raw_hex(stderr, stack_at(stack, i), stack->element_sz);
		fprintf(stderr, "\n");
	}
}