#include "log.h"

#include <stdlib.h>
#include <stdio.h>

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

int main()
{
	char buffer_rule[256] = "";
	char buffer_test[256] = "";
	Regex reg = {buffer_rule, 0, 0, buffer_test, 0};
	LOG("%p", buffer_rule)
	LOG("%p", buffer_test)
	while (1)
	{
		char op[256] = "";
		scanf("%s", op);
		LOG("%d", (int)op[0])
		if (op[0] == 'q')
			return 0;
		if (op[0] == 'r')
			scanf("%s", buffer_rule);
		else if (op[0] == 't')
			scanf("%s", buffer_test);
		else
			printf("result is: %d\n", regex(&reg));
	}
}