#include "list.h"

#include <stdio.h>

void out(List *list)
{
	printf("Printer:\n");
	for (size_t iter = list->front; iter != NONE; iter = list_right(list, iter))
	{
		printf("\t[%3.3lu]: %d\n", iter, *(int *)list_element(list, iter));
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

	list_dump(stdout, list);
	out(list);

	// 0 1 2 3 4 5 6 7 8 9
	for (int i = 0; i < 10; ++i)
	{
		*(int *)list_element(list, list_push_back(list)) = i;
		// list_dump(stderr, list);
	}
	out(list);

	// 0 1 2 3 4 5 6 7 9
	list_erase(list, list_left(list, list->back));
	out(list);

	list_at(list, 0);
	out(list);

	list_dump(stdout, list);
	graph(list);

	list_destruct(list);
}

int main()
{
	test_1();
}