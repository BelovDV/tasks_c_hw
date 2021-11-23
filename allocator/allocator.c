#include "allocator.h"
#include "utility.h"
#include "check.h"

#include <stdint.h>

#define PAGE_SIZE 4096
#define MAX_ALLOCATE 256
#define COUNT_ALIGN 6 // 8 16 32 64 128 256
#define PTR_SZ sizeof(void *)
#define EL_SZ(align) (1 << (align + 3)) // element size (align)

struct Iter_;
struct Iter_
{
	struct Iter_ *next;
};
typedef struct Iter_ Iter;

typedef struct
{
	size_t size;
	size_t capacity;
	void **data;
} Storage;

static Iter *ready[COUNT_ALIGN] = {0, 0, 0, 0, 0, 0};
static Storage storage = {0, 0, 0};
static uint8_t my_align[MAX_ALLOCATE + 1];

static void destruct()
{
	while (storage.size-- != 0)
		free(storage.data[storage.size]);
}

static void create_page(size_t align)
{
	if (storage.size == storage.capacity)
	{
		size_t new_cap = (storage.capacity + 2) * 2;
		storage.data = realloc(storage.data, new_cap * PTR_SZ);
		check_message(storage.data, check_calloc_failed, new_cap * PTR_SZ);
		storage.capacity = new_cap;
	}
	uint8_t *array = wrapped_calloc(1, PAGE_SIZE);
	for (size_t position = 0; position < PAGE_SIZE; position += EL_SZ(align))
	{
		Iter *pos = (Iter *)(array + position);
		pos->next = (ready[align]); // TODO - remove ()
		ready[align] = pos;
	}
	storage.data[storage.size++] = array;
}

void *allocate(size_t size)
{
	static int init = 1;
	if (init)
	{
		init = 0;
		atexit(destruct);
	}

	size_t align = my_align[size];
	if (!ready[align])
		create_page(align);

	void *result = (void *)ready[align];
	ready[align] = ready[align]->next;
	return result;
}

void deallocate(void *ptr, size_t size)
{
	size_t align = my_align[size];
	Iter *iter = ptr;
	iter->next = ready[align];
	ready[align] = iter;
}

static uint8_t my_align[] =
	{
		0,
		0,												// 1
		0,												// 2
		0, 0,											// 3 - 4
		0, 0, 0, 0,										// 5 - 8
		1, 1, 1, 1, 1, 1, 1, 1,							// 9 - 16
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 17 - 32
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 33 - 64
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // 65 - 128
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, // 129 - 256
};

#include <stdio.h>

#define ALIGN 2

static FILE *stream = NULL;

static void d_link_node_null(size_t number, size_t size)
{
	fprintf(stream,
			"\tstruct%lu:x%lux%lu -> NULL\n",
			number, number, size);
}

static void d_link_nodes(
	size_t number_1, size_t size_1,
	size_t number_2, size_t size_2)
{
	fprintf(stream,
			"\tstruct%lu:x%lux%lu -> struct%lu:x%lux%lu\n",
			number_1, number_1, size_1, number_2, number_2, size_2);
}

static void d_struct_page(size_t number, size_t size, size_t align)
{
	fprintf(stream, "\tstorage -> struct%lu:Page%lu\n", number, number);
	fprintf(
		stream, "\tstruct%lu [label=\"<Page%lu> Page %u %lu|{",
		number, number, EL_SZ(align), number);

	for (size_t i = 0; i < size; ++i)
	{
		if (i)
			fprintf(stream, "|");
		fprintf(stream, "<x%lux%lu> %lu", number, i, i);
	}

	fprintf(stream, "}\"]\n");
}

static size_t d_base(Iter *iter)
{
	for (size_t try = 0; try < storage.size; ++try)
		if ((uint8_t *)iter >= (uint8_t *)storage.data[try] &&
			(uint8_t *)iter < (uint8_t *)storage.data[try] + PAGE_SIZE)
			return try;
	return -1;
}

static size_t d_position(Iter *iter, size_t base, size_t align)
{
	return ((uint8_t *)iter - (uint8_t *)storage.data[base]) / EL_SZ(align);
}

static void d_tree(size_t align, char *visited)
{
	if (ready[align])
	{
		size_t base = d_base(ready[align]);
		fprintf(
			stream, "\tstart_%u -> struct%lu:x%lux%lu\n",
			EL_SZ(align), base, base, d_position(ready[align], base, align));
	}
	else
		fprintf(stream, "\tstart_%u -> NULL\n", EL_SZ(align));
	char *use = wrapped_calloc(storage.size, 1);
	for (Iter *iter = ready[align]; iter != NULL; iter = iter->next)
	{
		size_t base = d_base(iter);
		use[base] = 1;
		visited[base] = 1;
		if (!iter->next)
			d_link_node_null(base, d_position(iter, base, align));
		else
		{
			size_t base_next = d_base(iter->next);
			d_link_nodes(
				base, d_position(iter, base, align),
				base_next, d_position(iter->next, base_next, align));
		}
	}
	for (size_t iter = 0; iter < storage.size; ++iter)
		if (use[iter])
			d_struct_page(iter, PAGE_SIZE / EL_SZ(align), align);
	free(use);
}

#include <string.h>

void alloc_dump(const char *engine,
				const char *file_text, const char *file_image)
{
	debug_check(engine != NULL && file_image != NULL && file_text != NULL);
	check(
		strlen(engine) < 24 &&
		strlen(file_text) < 64 &&
		strlen(file_image) < 64);
	stream = wrapped_fopen(file_text, "w");
	fprintf(stream, "digraph Memory\n{\n");
	(void)file_text;
	(void)file_image;
	(void)engine;
	fprintf(stream, "\tnode [shape=record];\n\n");

	char *use = wrapped_calloc(storage.size, 1);
	for (size_t align = 0; align < COUNT_ALIGN; ++align)
	{
		fprintf(stream, "\tready -> start_%u\n", EL_SZ(align));
		d_tree(align, use);
	}
	for (size_t iter = 0; iter < storage.size; ++iter)
		if (!use[iter])
		{
			fprintf(stream, "\tPage_full_%lu\n", iter);
			fprintf(stream, "\tstorage -> Page_full_%lu\n", iter);
		}
	for (size_t iter = storage.size; iter < storage.capacity; ++iter)
	{
		fprintf(stream, "\tPage_not_allocated_%lu\n", iter);
		fprintf(stream, "\tstorage -> Page_not_allocated_%lu\n", iter);
	}
	free(use);
	fprintf(stream, "\tready [style=\"filled\", fillcolor=\"#aaaaaa\"]\n");
	fprintf(stream, "\tstorage [style=\"filled\", fillcolor=\"#aaaaaa\"]\n");
	fprintf(stream, "\tNULL [style=\"filled\", fillcolor=\"#aaaaaa\"]\n");

	fprintf(stream, "}\n");
	fclose(stream);
	stream = NULL;
	char vsp[256];
	sprintf(vsp, "%s -v -Tpng -o %s %s", engine, file_image, file_text);
	system(vsp);
}