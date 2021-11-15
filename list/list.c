#include "list.h"

#include <stdlib.h>
#include <assert.h>

#define pages_count 64
static List_iterator *list_pages[pages_count];
static size_t current_page = 0;
static List_iterator free_root = {&free_root, &free_root, 0, 0};

static void free_pages()
{
	for (size_t i = 0; i < pages_count; ++i)
		free(list_pages[i]);
}

static void prepare_page()
{
	static int vsp = 1;
	if (vsp)
	{
		atexit(free_pages);
		vsp = 0;
	}
	assert(current_page != pages_count);
	static size_t page_size = 2;
	page_size = page_size * 3 / 2;

	list_pages[current_page] = calloc(page_size * 1024,
									  sizeof(List_iterator));
	for (size_t i = 0; i < page_size * 1024; ++i)
		list_insert_righter(&free_root, list_pages[current_page] + i);
	++current_page;
}

List_iterator *list_create(uint64_t data_1, uint64_t data_2)
{
	if (free_root.right == &free_root)
		prepare_page();
	List_iterator *result = free_root.right;
	list_erase(result);
	result->data_1 = data_1;
	result->data_2 = data_2;
	return result;
}

void list_remove(List_iterator *iter)
{
	assert(iter != NULL);
	list_erase(iter);
	list_insert_righter(&free_root, iter);
	iter->data_1 = 0;
	iter->data_2 = 0;
}

void list_erase(List_iterator *iter)
{
	assert(iter != NULL);
	if (iter->right)
		iter->right->left = iter->left;
	if (iter->left)
		iter->left->right = iter->right;
	iter->left = NULL;
	iter->right = NULL;
}

void list_insert_righter(List_iterator *left, List_iterator *right)
{
	assert(left != NULL);
	assert(right != NULL);
	right->left = left;
	right->right = left->right;
	left->right = right;
	if (right->right)
		right->right->left = right;
}