#include "executor.h"
#include "text.h"

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 3)
	{
		printf(
			"takes arguments:\n"
			"\t binary .dull path\n"
			"\t(optional) path to log file\n");
		return 1;
	}
	FILE *log = stderr;
	if (argc >= 3)
	{
		FILE *log_file = fopen(argv[2], "w");
		if (!log_file)
			printf("can't open \"%s\"\n", argv[2]);
		else
			log = log_file;
		setvbuf(log, NULL, _IONBF, 0);
	}

	Text_string input = text_read_file(argv[1]);

	int result = execute((Array_static_frame *)&input, log);
	if (result)
		printf(log, "\nexecution failed\n");

	free(input.value);
	fclose(log);
	return result;
}