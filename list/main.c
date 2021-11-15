#include "list.h"

#include <stdio.h>

void test_1()
{
	List_iterator *root = list_create(0, 0);
	root->left = root;
	root->right = root;
	for (int i = 0; i < 10; ++i)
	{
		List_iterator *next = list_create(i + 1, i + 1);
		list_insert_righter(root, next);
	}

	for (List_iterator *iter = root->right; iter != root; iter = iter->right)
	{
		printf("%lu\n", iter->data_1);
	}
}

int main()
{
	test_1();
}