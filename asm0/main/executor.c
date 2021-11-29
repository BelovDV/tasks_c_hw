#include "executor.h"
#include "text.h"

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 4)
	{
		printf(
			"takes arguments:\n"
			"\t binary .dull path\n"
			"\t(optional) path to log file\n"
			"\t(optional) any if asm should ask for additional arguments\n"
			"\t\tfor assembling\n");
		return 1;
	}
	FILE *log = stderr;
	if (argc >= 3)
	{
		FILE *log_file = fopen(argv[2], "w");
		if (!log_file)
			fprintf(log, "can't open \"%s\"\n", argv[2]);
		else
			log = log_file;
		setvbuf(log, NULL, _IONBF, 0);
	}
	if (argc >= 4)
	{
		fprintf(log, "asm doesn't takes additional arguments\n");
		// TODO - ask for additional arguments and pass them to asm
	}

	Text_string input_ = text_read_file(argv[1]);
	Array_static_frame input = {input_.value, input_.size};

	int result = execute(input, log);
	if (result)
	{
		fprintf(log, "\nExecution failed\n");
	}

	free(input_.value);
	fclose(log);
	return result;
}