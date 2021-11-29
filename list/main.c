#include "list.h"

#include <stdio.h>

void out(List *list, size_t iterator)
{
	printf("Printer:\n");
	while (iterator != NONE)
	{
		printf("\t%d\n", *(int *)list_element(list, iterator));
		iterator = list_right(list, iterator);
	}
}

void vsp(void *stream, void *data_)
{
	int *data = data_;
	fprintf(stream, "%d", *data);
}

void graph(List *list)
{
	FILE *graph = fopen("../temp/list/graph", "w");
	if (!graph)
		return;
	list_graph(graph, list, vsp);
	fclose(graph);
}

void test_1()
{
	List list_value;
	List *list = &list_value;
	list_initialize(list, sizeof(int));

	Index a[10];
	list_dump(stdout, list);
	for (int i = 0; i < 10; ++i)
		a[i] = list_new(list);
	list_dump(stdout, list);
	for (int i = 0; i < 10; ++i)
		*(int *)list_element(list, a[i]) = i;
	for (int i = 0; i < 10; ++i)
		out(list, a[i]);

	// 2 4
	list_insert_after(list, a[2], a[4]);
	list_dump(stdout, list);
	out(list, a[2]);

	// 7 6 3
	list_insert_after(list, a[7], a[6]);
	list_insert_after(list, a[6], a[3]);
	list_dump(stdout, list);
	out(list, a[7]);

	// 1 5 8 9
	list_insert_before(list, a[9], a[8]);
	list_insert_before(list, a[8], a[5]);
	list_insert_before(list, a[5], a[1]);
	list_dump(stdout, list);
	out(list, a[1]);

	// 7 3
	list_erase(list, a[6]);
	list_dump(stdout, list);
	out(list, a[7]);

	graph(list);

	list_destruct(list);
}

void test_2()
{
}

int main()
{
	test_1();
	test_2();
}