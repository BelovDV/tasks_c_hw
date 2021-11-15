#include "stack.h"

#include <stdio.h>
#include <assert.h>
#include <errno.h>

void test_1()
{
	Stack_structure stack = {0, 0, 0, 0, 0};
	stack_initialise(&stack, sizeof(int));

	stack_dump(&stack);

	for (int i = 100; i < 130; ++i)
	{
		//stack_dump(&stack);
		stack_push(&stack, &i);
		printf("%d\n", *(int *)stack_top(&stack));
	}

	stack_dump(&stack);

	for (int i = 0; i < 27; ++i)
	{
		stack_pop(&stack);
		printf("%d\n", *(int *)stack_top(&stack));
	}

	stack_dump(&stack);

	stack_destruct(&stack);
}

void test_2()
{
	Stack_structure stack = {0, 0, 0, 0, 0};
	stack_initialise(&stack, sizeof(int));

	stack_pop(&stack);
	assert(errno == stack_err_empty_pop);
	printf("stack_err_empty_pop \tdone\n");

	stack_top(&stack);
	assert(errno == stack_err_empty_top);
	printf("stack_err_empty_top \tdone\n");

	stack_push(NULL, NULL);
	assert(errno == stack_err_null_stack);
	printf("stack_err_null_stack \tdone\n");

	stack_push(&stack, NULL);
	assert(errno == stack_err_null_element);
	printf("stack_err_null_element \tdone\n");

	stack.hash = 15;
	stack_top(&stack);
	assert(errno == stack_err_wrong_hash);
	printf("stack_err_wrong_hash \tdone\n");

	stack_destruct(&stack);
}

int main()
{
	test_1();

	test_2();
}