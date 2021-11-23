#include "list.h"

#include <assert.h>

struct Node_;
struct Node_
{
	struct Node_ *left;
	struct Node_ *right;
};
typedef struct Node_ Node;

void list_initialize(void *root_)
{
	Node *root = root_;
	root->left = root->right = root;
}

void list_destruct(void *root_, void (*deallocate)(void *))
{
	Node *root = root_;
	assert(root != NULL && deallocate != NULL);
	while (root->right != root)
		list_remove(root->right, deallocate);
	list_remove(root, deallocate);
}

int list_check_cycle(void *root_, size_t max_size)
{
	Node *root = root_;
	assert(root != NULL && max_size != 0);
	if (root->right == NULL || root->right->left != root)
		return 0;
	for (Node *iter = root->right; iter != root; iter = iter->right)
	{
		if (iter->right == NULL || iter->right->left != iter)
			return 0;
		if (max_size-- == 0)
			return 0;
	}
	return 1;
}

void list_link(void *left_, void *right_)
{
	Node *left = left_;
	Node *right = right_;
	if (left != NULL)
		left->right = right;
	if (right != NULL)
		right->left = left;
}

void list_insert_before(void *iter_, void *new_)
{
	Node *iter = iter_;
	Node *new = new_;
	assert(iter != NULL);
	if (!new)
		return;
	Node *left = iter->left;
	list_link(left, new);
	list_link(new, iter);
}

void list_insert_after(void *iter_, void *new)
{
	Node *iter = iter_;
	assert(iter != NULL);
	if (!new)
		return;
	Node *right = iter->right;
	list_link(iter, new);
	list_link(new, right);
}

void list_fetch(void *iter_)
{
	Node *iter = iter_;
	assert(iter != NULL);
	Node *left = iter->left;
	Node *right = iter->right;
	list_link(left, right);
	iter->left = iter->right = NULL;
}

void list_remove(void *iter, void (*deallocate)(void *))
{
	assert(deallocate != NULL);
	list_fetch(iter);
	deallocate(iter);
}