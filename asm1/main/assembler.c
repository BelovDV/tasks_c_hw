#include "assembler.h"
#include "utility.h"
#include "check.h"

int main(int argc, char **argv)
{
	if (argc < 3 || argc > 4)
	{
		printf(
			"takes arguments:\n"
			"\tsource code path\n"
			"\tdestination .dul path\n"
			"\t(optional) path to log file\n");
		return 1;
	}
	FILE *log = stderr;
	if (argc >= 4)
	{
		FILE *log_file = fopen(argv[3], "w");
		if (!log_file)
			printf("can't open \"%s\"\n", argv[3]);
		else
			log = log_file;
		setvbuf(log, NULL, _IONBF, 0);
	}

	Text_string input = text_read_file(argv[1]);
	Array_static_frame result = {0, 0};
	int was_error = assemble(&input, &result, log);
	if (was_error)
		printf("\nassembling failed\n");
	else
		text_write_raw_file(argv[2], (Text_string *)&result);

	free(input.value);
	fclose(log);
	return was_error;
}