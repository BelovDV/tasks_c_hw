#include "list.h"

#include <assert.h>

#include <stdio.h>

typedef struct
{
	Index right;
	Index left;
} Node;

/**
 * @brief size of full element
 */
static size_t size(List *list)
{
	return sizeof(Node) + list->element_size;
}

/**
 * @brief node from offset
 */
static Node *at(List *list, size_t offset)
{
	if (offset >= list->capacity)
		return NULL;
	return (Node *)((char *)list->array + offset);
}

/**
 * @brief offset from node
 */
static size_t offset(List *list, Node *node)
{
	return (char *)node - (char *)list->array;
}

void list_initialize(List *list, size_t element_size)
{
	assert(list != NULL);
	list->array = NULL;
	list->capacity = 0;
	list->free_position = 0;
	list->element_size = element_size;
}

void list_destruct(List *list)
{
	assert(list != NULL);
	free(list->array);
	list->array = NULL;
	list->capacity = 0;
	list->free_position = 0;
}

size_t list_new(List *list)
{
	assert(list != NULL);
	if (at(list, list->free_position) == NULL)
	{
		size_t capacity = list->capacity ? list->capacity * 2 : size(list) * 4;
		void *array = realloc(list->array, capacity);
		if (!array)
			return NONE;
		list->array = array;

		size_t iter = list->capacity;
		list->free_position = capacity;
		list->capacity = capacity;
		for (; iter != capacity; iter += size(list))
		{
			at(list, iter)->right = list->free_position;
			at(list, iter)->left = NONE;
			list->free_position = iter;
		}
	}
	Node *new = at(list, list->free_position);
	if (new == NULL)
	{
		printf("ERROR\n");
		return 0;
	}
	list->free_position = new->right;
	new->right = new->left = NONE;
	return offset(list, new);
}

size_t list_right(List *list, size_t element)
{
	assert(list != NULL);
	Node *node = at(list, element);
	return node->right;
}

size_t list_left(List *list, size_t element)
{
	assert(list != NULL);
	Node *node = at(list, element);
	return node->left;
}

void list_insert_after(List *list, size_t guideline, size_t inserting)
{
	assert(list != NULL);
	Node *guide = at(list, guideline);
	Node *new = at(list, inserting);
	new->left = offset(list, guide);
	new->right = guide->right;
	if (guide->right != NONE)
		at(list, guide->right)->left = offset(list, new);
	guide->right = offset(list, new);
}

void list_insert_before(List *list, size_t guideline, size_t inserting)
{
	assert(list != NULL);
	Node *guide = at(list, guideline);
	Node *new = at(list, inserting);
	new->right = offset(list, guide);
	new->left = guide->left;
	if (guide->left != NONE)
		at(list, guide->left)->right = offset(list, new);
	guide->left = offset(list, new);
}

void list_erase(List *list, size_t element)
{
	assert(list != NULL);
	Node *node = at(list, element);
	at(list, node->left)->right = node->right;
	at(list, node->right)->left = node->left;
	node->left = NONE;
	node->right = NONE;
}

void *list_element(List *list, Index element)
{
	return at(list, element) + 1;
}

void list_dump(void *stream, List *list)
{
	fprintf(stream, "DUMP list:\n");
	fprintf(stream, "\tcapacity: %lu\n\toffset: %lu\n",
			list->capacity, list->free_position);
	for (size_t i = 0; i < list->capacity; i += size(list))
		fprintf(stream, "   %3lu: next: %18lu; prev: %18lu\n",
				i / size(list),
				at(list, i)->right / size(list),
				at(list, i)->left / size(list));
	fprintf(stream, "\toffset:\n");
	Node *iter = at(list, list->free_position);
	while (iter != NULL)
	{
		fprintf(stream, "\t\t%lu\n", offset(list, iter) / size(list));
		iter = at(list, iter->right);
	}
}

void list_graph(void *stream, List *list, void (*printer)(void *, void *))
{
	assert(stream != NULL);
	assert(list != NULL);
	fprintf(stream, "digraph List\n{\n");

	for (size_t i = 0; i < list->capacity; i += size(list))
	{
		Node *cur = at(list, i);
		fprintf(stream, "\tnode%lu", i / size(list));
		if (printer)
		{
			fprintf(stream, "[label=");
			printer(stream, cur + 1);
			fprintf(stream, "]\n");
		}
		else
			fprintf(stream, "\n");
		if (cur->right != NONE && cur->right != list->capacity)
			fprintf(stream, "\tnode%lu->node%lu[label=right]\n",
					i / size(list), cur->right / size(list));
		if (cur->right == list->capacity)
			fprintf(stream, "\tnode%lu->END[label=right]\n",
					i / size(list));
		if (cur->left != NONE)
			fprintf(stream, "\tnode%lu->node%lu[label=left]\n",
					i / size(list), cur->left / size(list));
	}

	fprintf(stream, "}\n");
}