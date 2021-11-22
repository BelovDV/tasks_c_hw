#include "list.h"

#include <stdio.h>

void printer(List_iterator *root)
{
	printf("p\n");
	for (List_iterator *iter = root->right; iter != root; iter = iter->right)
		printf("%lu\n", iter->data_1);
}

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
	printer(root);

	printf("Insert 15 after 9\n");
	List_iterator *iter = root->right->right;
	list_insert_righter(iter, list_create(15, 15));
	printer(root);

	printf("Erase 7\n");
	iter = root->right->right->right->right->right;
	//list_erase(iter);
	list_remove(iter);
	printer(root);

	printf("Insert 31 after 9 and 32 after it\n");
	iter = root->right->right;
	list_insert_righter(iter, list_create(31, 31));
	iter = iter->right;
	list_insert_righter(iter, list_create(32, 32));
	printer(root);

	list_destructor(root);
}

void test_2()
{
	printf("TEST 2\n");

	List_iterator *left = list_create(0, 0);
	List_iterator *right = list_create(1, 1);

	printer(left);
	printer(right);

	list_insert_righter(left, list_create(10, 10));
	list_insert_righter(right, list_create(11, 11));

	List_iterator *result = list_join(left, right);
	printer(result);
	list_destructor(result);
}

int main()
{
	for (int i = 0; i < 10; ++i)
	{
		printf("TEST 1 %d\n", i);
		test_1();
	}
	test_2();
}