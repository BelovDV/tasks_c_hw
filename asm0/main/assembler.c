#include "assembler.h"
#include "utility.h"

int main(int argc, char **argv)
{
	if (argc < 3 || argc > 5)
	{
		printf(
			"takes arguments:\n"
			"\tsource code path\n"
			"\tdestination .dul path\n"
			"\t(optional) path to log file\n"
			"\t(optional) any if asm should ask for additional arguments\n"
			"\t\tfor assembling\n");
		return 1;
	}
	FILE *log = stderr;
	if (argc >= 4)
	{
		FILE *log_file = fopen(argv[3], "r");
		if (!log_file)
			fprintf(log, "can't open \"%s\"\n", argv[3]);
		else
			log = log_file;
	}
	if (argc >= 5)
	{
		fprintf(log, "asm doesn't takes additional arguments\n");
		// TODO - ask for additional arguments and pass them to asm
	}

	Text_string_static input = text_read_file(argv[1]);

	struct Dull *result = NULL;
	size_t size = 0;

	if (assemble(input, &result, log, &size))
		printf("assembling failed\n");
	else
	{
		FILE *output = wrapped_fopen(argv[2], "w");
		fwrite(result, 1, size, output);
		fclose(output);
		free(result);
	}
	free(input.string);
	fclose(log);
}