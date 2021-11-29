#include "assembler.h"
#include "utility.h"
#include "check.h"

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
		FILE *log_file = fopen(argv[3], "w");
		if (!log_file)
			fprintf(log, "can't open \"%s\"\n", argv[3]);
		else
			log = log_file;
		setvbuf(log, NULL, _IONBF, 0);
	}
	if (argc >= 5)
	{
		fprintf(log, "asm doesn't takes additional arguments\n");
		// TODO - ask for additional arguments and pass them to asm
	}

	Text_string input = text_read_file(argv[1]);

	Array_static_frame result = {0, 0};

	int was_error = assemble(input, &result, log);
	if (was_error)
		printf("assembling failed\n");
	else
	{
		FILE *output = wrapped_fopen(output, argv[2], "w");
		fwrite(result.array, 1, result.size, output);
		fclose(output);
		free(result.array);
	}
	free(input.value);
	fclose(log);
	return was_error;
}