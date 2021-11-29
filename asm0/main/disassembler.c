#include "disassembler.h"
#include "check.h"

int main(int argc, char **argv)
{
	if (argc < 3 || argc > 5)
	{
		printf(
			"takes arguments:\n"
			"\t binary .dull path\n"
			"\t destination code path\n"
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

	Text_string input_ = text_read_file(argv[1]);
	Array_static_frame input = {input_.value, input_.size};

	FILE *wrapped_fopen(output, argv[2], "w");
	int result = disassemble(input, output, log);
	if (result)
		fprintf(output, "\nDisassembling failed\n");
	fclose(output);

	free(input_.value);
	fclose(log);
	return result;
}