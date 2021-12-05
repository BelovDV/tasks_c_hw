#include "executor.h"
#include "language.h"
#include "log.h"
#include "dull.h"
#include "check.h"
#include "utility.h"
#include "coder.h"

#include <string.h>

static int process(Executor *exe);
static int log_error(Executor *exe, const char *message);

#define RIP (exe->r[e_exe_reg_rip])
#define RBP (exe->r[e_exe_reg_rbp])
#define RSP (exe->r[e_exe_reg_rsp])
#define RF (exe->r[e_exe_reg_rf])
#define CODE ((char *)exe->memory + RIP)

#define CHECK_RETURN(condition, message) \
	{                                    \
		if (!(condition))                \
		{                                \
			log_error(exe, message);     \
			LOG_OUT                      \
			return 1;                    \
		}                                \
	}

int execute(Array_static_frame *code, FILE *log)
{
	debug_check(code != NULL && code->array != NULL && log != NULL);
	check_message(code->size > sizeof(struct Dull),
				  "EXECUTOR: too little input file%s", "\n");
	LOG_IN
	LOG("0x%lx", MEMORY_SIZE)
	LOG("0x%lx", STACK_OFFSET)
	LOG("0x%lx", STACK_SIZE)
	LOG("0x%lx", CODE_OFFSET)
	LOG("0x%lx", CODE_SIZE)
	fprintf(log, "Executor began working\n");
	Executor *exe = calloc(1, sizeof(*exe));
	struct Dull *dull = code->array;
	RIP = dull->start_offset + CODE_OFFSET;
	LOG("0x%lx", dull->start_offset)
	RBP = STACK_OFFSET;
	RSP = STACK_OFFSET;
	exe->memory = calloc(1, MEMORY_SIZE);
	LOG("%p", exe->memory)
	exe->log = log;
	memcpy((char *)exe->memory + CODE_OFFSET, dull + 1,
		   code->size - sizeof(*dull));
	int is_error = process(exe);
	if (is_error)
	{
		executor_dump(exe->log, exe, 0);
		fprintf(exe->log, "\nErrors were encountered\n");
	}
	fprintf(log, "Executor ended working\n");
	free(exe->memory);
	free(exe);
	LOG_OUT
	return is_error;
}

static int process(Executor *exe)
{
	LOG_IN
	while (!(exe->r[e_exe_reg_rf] & (e_exe_rf_end | e_exe_rf_error)))
	{
#ifdef EXTRA_LOG
		executor_dump(exe->log, exe, 0);
#endif
		LOG("0x%lx", RIP)
		CHECK_RETURN(RIP >= CODE_OFFSET && RIP < CODE_OFFSET + CODE_SIZE,
					 "try to execute nonexecutable memory")
		size_t sz = coder_decode(CODE, &exe->instr);
		CHECK_RETURN(sz != 0 && exe->instr.type == e_line_instr,
					 "cannot decode instruction")
		LOG("%s", mnemonics[exe->instr.instr.id])
		RIP += sz;
		CHECK_RETURN(!language_execute(exe), "cannot execute instr")
	}
	LOG_OUT
	return RF & e_exe_rf_error;
}

static int log_error(Executor *exe, const char *message)
{
	debug_check(exe != NULL && message != NULL && exe->log != NULL);
	fprintf(exe->log, "ERROR: rip 0x");
	fprintf(exe->log, "%lx: ", RIP);
	fprintf(exe->log, "%s\n", message);
	return 0;
}

void executor_dump(FILE *stream, void *executor, int big)
{
	Executor *exe = executor;
	if (big)
	{
		fprintf(stream, "DUMP: Executor\n");
	}
	else
	{
		fprintf(stream, "Exe: rip=0x%12.12lx\n", exe->r[e_exe_reg_rip]);
		for (int i = 0; i < 16; ++i)
			fprintf(stream, "%12.12lu ", exe->r[i]);
		fprintf(stream, "\n");
	}
}