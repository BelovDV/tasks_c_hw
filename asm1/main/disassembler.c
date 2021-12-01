#include "disassembler.h"
#include "check.h"

int main(int argc, char **argv)
{
	if (argc < 3 || argc > 5)
	{
		printf(
			"takes arguments:\n"
			"\tbinary .dull path\n"
			"\tdestination code path\n"
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
	FILE *output = fopen(argv[2], "w");
	if (!output)
		printf("cannot open file '%s'\n", argv[2]);

	Text_string input_ = text_read_file(argv[1]);
	Array_static_frame input = {input_.value, input_.size};
	int result = disassemble(&input, output, log);
	if (result)
		printf("\ndisassembling failed\n");
	fclose(output);

	free(input_.value);
	fclose(log);
	return result;
}