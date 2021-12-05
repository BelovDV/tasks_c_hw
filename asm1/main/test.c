#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
	char *rule;				// pointer to main rule
	char **additional_rule; // pointer to set of rules
	char **result;			// pointer to dest of '?', should be allocated
	char *considered;		// string which one to process

	int flags; // inner
} Regex;

/**
 * @brief parse string 'iter' by rules 'rule' and save result
 * @return is_error
 */
int regex(Regex *regex);

void test_1()
{
	char buffer_rule[256] = "";
	char buffer_test[256] = "";
	Regex reg = {buffer_rule, 0, 0, buffer_test, 0};
	LOG("%p", buffer_rule)
	LOG("%p", buffer_test)
	while (1)
	{
		reg.rule = buffer_rule;
		reg.considered = buffer_test;
		char op[256] = "";
		scanf("%s", op);
		LOG("%d", (int)op[0])
		if (op[0] == 'q')
			return;
		if (op[0] == 'r')
			scanf("%s", buffer_rule);
		else if (op[0] == 't')
			scanf("%s", buffer_test);
		else
			printf("result is: %d\n", regex(&reg));
	}
}

void test_2()
{
	char buffer_rule[256] = "r(!(1[0-1])|![0-9]|!(bp)|!(sp)|!f)";
	char buffer_test[256] = "";
	char *buffer_result[256];
	Regex reg = {buffer_rule, 0, 0, buffer_test, 0};
	while (1)
	{
		reg.rule = buffer_rule;
		reg.considered = buffer_test;
		reg.result = buffer_result;

		LOG("%p", reg.result)

		scanf("%s", buffer_test);
		if (strcmp(buffer_test, "q") == 0)
			return;
		printf("result is: %d\n", regex(&reg));
		for (int i = 0; i < 5; ++i)
		{
			printf("%p\t", buffer_result + i);
			printf("%p\n", buffer_result[i]);
		}
		printf("\n");
	}
}

void test_3()
{
	// char buffer_rule[256] = "r(?(1[0-1])|?[0-9]|?(bp)|?(sp)|?f)";
	char buffer_rule[] =
		"?$["
		"?(r(!(1[0-1])|![0-9]|!(bp)|!(sp)|!f))"
		"?+"
		"?((!1|!2|!4|!8)$*!r(!(1[0-1])|![0-9]|!(bp)|!(sp)|!f))"
		"?+"
		"?([1-9]*[0-9])"
		"?$]";
	/*
	char buffer_rule[] =
		"?$["
		"?(r(!(1[0-1])|![0-9]|!(bp)|!(sp)|!f))"
		"?+"
		"?$]";
		*/
	char buffer_test[256] = "";
	char *buffer_result[256];
	Regex reg = {buffer_rule, 0, 0, buffer_test, 0};
	while (1)
	{
		reg.rule = buffer_rule;
		reg.considered = buffer_test;
		reg.result = buffer_result;

		LOG("%p", reg.result)

		scanf("%s", buffer_test);
		if (strcmp(buffer_test, "q") == 0)
			return;
		printf("result is: %d\n", regex(&reg));

		printf("%20.20s: %p\n", "is [", buffer_result[0]);
		printf("%20.20s: %p\n", "is rx", buffer_result[1]);
		printf("%20.20s: %p\n", "does rx = r1.", buffer_result[2]);
		printf("%20.20s: %p\n", "does rx = r.", buffer_result[3]);
		printf("%20.20s: %p\n", "does rx = rbp", buffer_result[4]);
		printf("%20.20s: %p\n", "does rx = rsp", buffer_result[5]);
		printf("%20.20s: %p\n", "does rx = rf", buffer_result[6]);
		printf("%20.20s: %p\n", "is + after rx", buffer_result[7]);
		printf("%20.20s: %p\n", "is k", buffer_result[8]);
		printf("%20.20s: %p\n", "does k = 1", buffer_result[9]);
		printf("%20.20s: %p\n", "does k = 2", buffer_result[10]);
		printf("%20.20s: %p\n", "does k = 4", buffer_result[11]);
		printf("%20.20s: %p\n", "does k = 8", buffer_result[12]);
		printf("%20.20s: %p\n", "is ry.", buffer_result[13]);
		printf("%20.20s: %p\n", "does ry = r1.", buffer_result[13]);
		printf("%20.20s: %p\n", "does ry = r.", buffer_result[14]);
		printf("%20.20s: %p\n", "does ry = rbp", buffer_result[15]);
		printf("%20.20s: %p\n", "does ry = rsp", buffer_result[16]);
		printf("%20.20s: %p\n", "does ry = rf", buffer_result[17]);
		printf("%20.20s: %p\n", "is + after ry", buffer_result[18]);
		printf("%20.20s: %p\n", "is c", buffer_result[19]);
		printf("%20.20s: %p\n", "is ]", buffer_result[20]);

		printf("\n");
	}
}

int main()
{
	int val;
	scanf("%d", &val);
	if (val == 1)
		test_1();
	if (val == 2)
		test_2();
	if (val == 3)
		test_3();
}