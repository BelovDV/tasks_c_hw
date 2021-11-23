#include "list.h"
#include "allocator.h"

struct Iter_;
struct Iter_
{
	struct Iter_ *left;
	struct Iter_ *right;
	int data;
};
typedef struct Iter_ Iter;

void out(Iter *root)
{
	printf("List:\n");
	for (Iter *iter = root->right; iter != root; iter = iter->right)
		printf("\t%d\n", iter->data);
}

void deallocate_iter(void *ptr)
{
	deallocate(ptr, sizeof(Iter));
}

void test_1()
{
	printf("Test:\n"
		   " initialize\n"
		   " destruct\n"
		   " check_cycle\n"
		   " insert_before\n"
		   " insert_after\n"
		   " remove\n");

	Iter *list = allocate(sizeof(*list));
	list_initialize(list);
	out(list);

	printf("Check list_check_cycle true: %d\n", list_check_cycle(list, 100));

	list->left = NULL;
	printf("Check list_check_cycle false: %d\n", list_check_cycle(list, 100));
	list->left = list;

	for (size_t i = 0; i < 9; ++i)
	{
		Iter *new = allocate(sizeof(Iter));
		new->data = i;
		list_insert_after(list, new);
	}
	printf("Check: 8 7 6 5 4 3 2 1 0:\n");
	out(list);

	printf("Insert 5 before 7:\n");
	Iter *new = allocate(sizeof(Iter));
	new->data = 5;
	list_insert_before(list->right->right, new);
	out(list);

	printf("Remove 7:\n");
	list_remove(list->right->right->right, deallocate_iter);
	out(list);

	list_destruct(list, deallocate_iter);
}

#define C16 16
#define C32 32

void test_2()
{
	system("echo hello!");

	void *ar_16[C16];
	for (size_t i = 0; i < C16; ++i)
		ar_16[i] = allocate(16);

	void *ar_32[C32];
	for (size_t i = 0; i < C32; ++i)
		ar_32[i] = allocate(32);

	void *_ar_32[C32];
	for (size_t i = 0; i < C32; ++i)
		_ar_32[i] = allocate(32);

	for (size_t i = 0; i < C16; ++i)
		deallocate(ar_16[i], 16);

	for (size_t i = 0; i < C32 - 1; ++i)
		deallocate(ar_32[i], 32);

	deallocate(_ar_32[C32 - 1], 32);
	deallocate(_ar_32[C32 - 6], 32);
	deallocate(ar_32[C32 - 1], 32);

	deallocate(allocate(1), 1);
	deallocate(allocate(128), 128);

	alloc_dump("dot", "graph", "img");

	for (size_t i = 0; i < C32 - 1; ++i)
		if (i != C32 - 6)
			deallocate(_ar_32[i], 32);
}

int main()
{
	printf("\t\t\t\tTEST 1:\n");
	test_1();

	printf("\t\t\t\tTEST 2:\n");
	test_2();
}