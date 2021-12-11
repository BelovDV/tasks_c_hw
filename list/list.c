#include "list.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <stdio.h>

typedef struct
{
	Index right;
	Index left;
} Node;

#define check assert(list != NULL && element <= list->capacity - size(list));

/**
 * @brief size of full element
 */
static Index size(List *list)
{
	return sizeof(Node) + list->element_size;
}

/**
 * @brief node from offset
 */
static Node *at(List *list, Index offset)
{
	if (offset + size(list) > list->capacity)
		return NULL;
	return (Node *)((char *)list->array + offset);
}

/**
 * @brief offset from node
 */
static Index offset(List *list, Node *node)
{
	return (char *)node - (char *)list->array;
}

void list_initialize(List *list, size_t element_size)
{
	assert(list != NULL);
	list->array = NULL;
	list->capacity = 0;
	list->front = NONE;
	list->back = NONE;
	list->_is_index = 1;
	list->element_size = ((element_size + 7) & ~7lu);
}

void list_destruct(List *list)
{
	assert(list != NULL);
	free(list->array);
	list->array = NULL;
	list->capacity = 0;
	list->front = NONE;
	list->back = NONE;
}

static Node *list_new(List *list)
{
	assert(list != NULL);
	if (at(list, list->_free) == NULL)
	{
		Index capacity = list->capacity ? list->capacity * 2 : size(list) * 4;
		void *array = realloc(list->array, capacity);
		if (!array)
			return NULL;
		list->array = array;

		Index iter = list->capacity;
		list->_free = capacity;
		list->capacity = capacity;
		for (; iter != capacity; iter += size(list))
		{
			at(list, iter)->right = list->_free;
			at(list, iter)->left = NONE;
			if (list->_free < capacity)
				at(list, list->_free)->left = iter;
			list->_free = iter;
		}
	}
	Node *new = at(list, list->_free);
	assert(new != NULL);
	list->_free = new->right;
	if (at(list, list->_free))
		at(list, list->_free)->left = NONE;
	new->right = new->left = NONE;
	return new;
}

Index list_right(List *list, Index element)
{
	check;
	Node *node = at(list, element);
	if (node)
		return node->right;
	return NONE;
}

Index list_left(List *list, Index element)
{
	check;
	Node *node = at(list, element);
	if (node)
		return node->left;
	return NONE;
}

Index list_insert_after(List *list, Index element)
{
	check;
	list->_is_index = 0;
	if (element > list->capacity - size(list))
		return NONE;
	Node *new = list_new(list);
	Node *guide = at(list, element);
	assert(guide != NULL);
	if (new == NULL)
		return NONE;
	new->left = offset(list, guide);
	new->right = guide->right;
	if (guide->right != NONE)
		at(list, guide->right)->left = offset(list, new);
	guide->right = offset(list, new);
	if (list->back == element)
		list->back = offset(list, new);
	return offset(list, new);
}

Index list_insert_before(List *list, Index element)
{
	check;
	list->_is_index = 0;
	if (element > list->capacity - size(list))
		return NONE;
	Node *guide = at(list, element);
	Node *new = list_new(list);
	if (!new)
		return NONE;
	new->right = offset(list, guide);
	new->left = guide->left;
	if (guide->left != NONE)
		at(list, guide->left)->right = offset(list, new);
	guide->left = offset(list, new);
	if (list->front == element)
		list->front = offset(list, new);
	return offset(list, new);
}

void list_erase(List *list, Index element)
{
	check;
	list->_is_index = 0;
	Node *node = at(list, element);
	at(list, node->left)->right = node->right;
	at(list, node->right)->left = node->left;
	node->left = NONE;
	node->right = list->_free;
	if (at(list, list->_free))
		at(list, list->_free)->left = offset(list, node);
	list->_free = element;
}

Index list_create_first(List *list)
{
	assert(list != NULL);
	if (list->front != NONE)
		return NONE;
	list->front = list->back = offset(list, list_new(list));
	return list->front;
}

Index list_push_back(List *list)
{
	assert(list != NULL);
	if (list->front == NONE)
		return list_create_first(list);
	return list_insert_after(list, list->back);
}

Index list_push_front(List *list)
{
	assert(list != NULL);
	if (list->front == NONE)
		return list_create_first(list);
	return list_insert_before(list, list->front);
}

void *list_element(List *list, Index element)
{
	check return at(list, element) + 1;
}

void list_dump(void *stream, List *list)
{
	fprintf(stream, "DUMP list:\n");
	fprintf(stream, "\tcapacity: %lu\n\toffset: %lu\n",
			list->capacity, list->_free);
	for (Index i = 0; i < list->capacity; i += size(list))
	{
		fprintf(stream, "   %3lu: next: %18lu; prev: %18lu",
				i / size(list),
				at(list, i)->right / size(list),
				at(list, i)->left / size(list));
		if (list->element_size)
			fprintf(stream, "\t0x%x",
					(unsigned)*(unsigned char *)(at(list, i) + 1));
		fprintf(stream, "\n");
	}
	fprintf(stream, "\tfree:\n");
	Node *iter = at(list, list->_free);
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

	for (Index i = 0; i < list->capacity; i += size(list))
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

static void swap(List *list, Node *a, Node *b)
{
	Index a_left = a->left;
	Index a_right = a->right;
	Index b_left = b->left;
	Index b_right = b->right;
	if (a_left != NONE)
		at(list, a_left)->right = offset(list, b);
	if (a_right != NONE && a_right != list->capacity)
		at(list, a_right)->left = offset(list, b);
	if (b_left != NONE)
		at(list, b_left)->right = offset(list, a);
	if (b_right != NONE && b_right != list->capacity)
		at(list, b_right)->left = offset(list, a);
	size_t *i_a = (size_t *)a;
	size_t *i_b = (size_t *)b;
	for (size_t i = 0; i < size(list) / sizeof(size_t); ++i)
	{
		size_t vsp = i_a[i];
		i_a[i] = i_b[i];
		i_b[i] = vsp;
	}
}

Index list_at(List *list, Index element)
{
	check;
	if (list->_is_index)
		return element * size(list);
	list->_is_index = 1;
	for (Index iter = list->front, position = 0; iter != NONE; ++position)
	{
		Index swapped = position * size(list);
		if (iter == swapped)
		{
			iter = list_right(list, swapped);
			continue;
		}
		swap(list, at(list, iter), at(list, swapped));
		iter = list_right(list, swapped);
	}
	list->front = 0;
	return element * size(list);
}