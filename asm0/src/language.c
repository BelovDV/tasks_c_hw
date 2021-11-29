#include "language.h"
#include "instruction.h"
#include "executor.h"
#include "check.h"

#include <stdint.h>
#include <math.h>

static int lang_error(void *exe_, void *instr_);
static int lang_nope(void *exe_, void *instr_);
static int lang_libcall(void *exe_, void *instr_);
static int lang_syscall(void *exe_, void *instr_);
static int lang_mov(void *exe_, void *instr_);
static int lang_dump(void *exe_, void *instr_);
static int lang_mode(void *exe_, void *instr_);
static int lang_sub(void *exe_, void *instr_);
static int lang_div(void *exe_, void *instr_);
static int lang_add(void *exe_, void *instr_);
static int lang_mul(void *exe_, void *instr_);
static int lang_jmp(void *exe_, void *instr_);
static int lang_call(void *exe_, void *instr_);
static int lang_ret(void *exe_, void *instr_);

Instruction_description language_instructions[e_lang_instr_count] =
	{
		{"error", lang_error},
		{"nope", lang_nope},
		{"libcall", lang_libcall},
		{"syscall", lang_syscall},
		{"mov", lang_mov},
		{"dump", lang_dump},
		{"mode", lang_mode},
		{"sub", lang_sub},
		{"div", lang_div},
		{"add", lang_add},
		{"mul", lang_mul},
		{"jmp", lang_jmp},
		{"call", lang_call},
		{"ret", lang_ret}};

char language_keywords[e_lang_key_count][e_lang_max_word_len] =
	{
		"exit",
		"write_num",
		"read_num",
		"write_char",
		"read_char",
		"write_str",
		"read_str",
		"sqrt"};

// ===== // ===== // ===== // ===== // ===== // ===== // ===== // ===== //

#define R(number) (exe->r[((number) + exe->shift_reg) & 15])
#define EXE_IS_R(r) (r >= 0 && r < 8)

#define CHECK(condition, error_id) \
	if (!(condition))              \
	{                              \
		exe->error |= error_id;    \
		return 1;                  \
	}

static Word *tr_args[e_const_max_arg_count] = {0, 0, 0};
static Word const_tr_args[e_const_max_arg_count] = {0, 0, 0};

// rx+k*ry+c | [rx+k*ry+c]
static int lang_parse(Executor *exe, Instruction *instr)
{
	LOG_L(instr->argc, "%d")
	for (int argn = 0; argn < instr->argc; ++argn)
	{
		Argument *arg = &instr->argv[argn];
		arg->content &= e_arg_is__mask;
		if (arg->content == e_arg_is_rx)
		{
			LOG_L(arg->rx, "%hhd")
			tr_args[argn] = &R(arg->rx);
			continue;
		}
		Word result = 0;
		if (arg->content & e_arg_is_rx)
			result += R(arg->rx);
		if (arg->content & e_arg_is_ry)
			result += arg->k * R(arg->ry);
		if (arg->content & e_arg_is_const)
			result += arg->c;
		if (arg->content & e_arg_is_memory)
		{
			CHECK(result < MEMORY_SIZE, e_exe_error_memory_access_violation)
			tr_args[argn] = (uint64_t *)((uint8_t *)exe->memory + result);
			return 0;
		}
		else
		{
			const_tr_args[argn] = result;
			tr_args[argn] = const_tr_args + argn;
		}
	}
	return 0;
}

#define EXE_BIT(exe) (3 & exe->r[e_exe_reg_rf])

// ===== // ===== // ===== // ===== // ===== // ===== // ===== // ===== //

#define PREPARE_CAST             \
	Executor *exe = exe_;        \
	Instruction *instr = instr_; \
	(void)exe;                   \
	(void)instr;

#define PREPARE_ARGS            \
	PREPARE_CAST                \
	if (lang_parse(exe, instr)) \
		return 1;

#define MASTER(function)   \
	function(0, uint8_t);  \
	function(1, uint16_t); \
	function(2, uint32_t); \
	function(3, uint64_t); \
	function(4, int8_t);   \
	function(5, int16_t);  \
	function(6, int32_t);  \
	function(7, int64_t);  \
	function(14, float);   \
	function(15, double);

#define MASTER_CASE(function)                        \
	switch (exe->r[e_exe_reg_rf] & e_exe_flags_word) \
	{                                                \
		MASTER(function)                             \
	default:                                         \
		exe->error |= e_exe_error_wrong_word_type;   \
		return 1;                                    \
	}

static int lang_error(void *exe_, void *instr_)
{
	PREPARE_CAST
	return 1;
}

static int lang_nope(void *exe_, void *instr_)
{
	PREPARE_CAST
	return 0;
}

static int lang_libcall(void *exe_, void *instr_)
{
	PREPARE_CAST

	switch (instr->argv[0].c)
	{
	case e_lang_key_write_num:
		switch (exe->r[e_exe_reg_rf] & e_exe_flags_word)
		{
		case 0:
			printf("%hhu", *(uint8_t *)&R(1));
			return 0;
		case 1:
			printf("%hu", *(uint16_t *)&R(1));
			return 0;
		case 2:
			printf("%u", *(uint32_t *)&R(1));
			return 0;
		case 3:
			printf("%lu", *(uint64_t *)&R(1));
			return 0;
		case 4:
			printf("%hhd", *(int8_t *)&R(1));
			return 0;
		case 5:
			printf("%hd", *(int16_t *)&R(1));
			return 0;
		case 6:
			printf("%d", *(int32_t *)&R(1));
			return 0;
		case 7:
			printf("%ld", *(int64_t *)&R(1));
			return 0;
		case 14:
			printf("%f", *(float *)&R(1));
			return 0;
		case 15:
			printf("%lf", *(double *)&R(1));
			return 0;
		default:
			exe->error |= e_exe_error_wrong_word_type;
			return 1;
		}
	case e_lang_key_read_num:
		switch (exe->r[e_exe_reg_rf] & e_exe_flags_word)
		{
		case 0:
			scanf("%hhu", (uint8_t *)&R(0));
			return 0;
		case 1:
			scanf("%hu", (uint16_t *)&R(0));
			return 0;
		case 2:
			scanf("%u", (uint32_t *)&R(0));
			return 0;
		case 3:
			scanf("%lu", (uint64_t *)&R(0));
			return 0;
		case 4:
			scanf("%hhd", (int8_t *)&R(0));
			return 0;
		case 5:
			scanf("%hd", (int16_t *)&R(0));
			return 0;
		case 6:
			scanf("%d", (int32_t *)&R(0));
			return 0;
		case 7:
			scanf("%ld", (int64_t *)&R(0));
			return 0;
		case 14:
			scanf("%f", (float *)&R(0));
			return 0;
		case 15:
			scanf("%lf", (double *)&R(0));
			return 0;
		default:
			exe->error |= e_exe_error_wrong_word_type;
			return 1;
		}
	case e_lang_key_write_char:
		printf("%c", (char)R(1));
		return 0;
	case e_lang_key_write_str:
	{
		Word address = R(1);
		CHECK(address < MEMORY_SIZE, e_exe_error_memory_access_violation)
		printf("%*s", (int)(MEMORY_SIZE - address), (uint8_t *)exe->memory + address);
		return 0;
	}
	case e_lang_key_read_char:
		scanf("%c", (char *)&R(1));
		return 0;
	case e_lang_key_read_str:
	{
		Word address = R(1);
		CHECK(address < MEMORY_SIZE - 1024, e_exe_error_memory_access_violation)
		// TODO
		// how to restrict scanf length with variable?
		scanf("%1024s", (char *)exe->memory + address);
		return 0;
	}
	case e_lang_key_sqrt:
	{
		if (*(double *)&R(1) < 0)
			return 1;
		*(double *)&R(0) = sqrt(*(double *)&R(1));
		return 0;
	}
	default:
		exe->error |= e_exe_error_wrong_keyword;
		return 1;
	}
}

static int lang_syscall(void *exe_, void *instr_)
{
	PREPARE_CAST

	switch (instr->argv[0].c)
	{
	case e_lang_key_exit:
		exe->r[e_exe_reg_rf] |= e_exe_flag_exit;
		return 0;
	default:
		exe->error |= e_exe_error_wrong_keyword;
		return 1;
	}
}

static int lang_mov(void *exe_, void *instr_)
{
	PREPARE_ARGS
	CHECK(instr->argc == 1 || instr->argc == 2, e_exe_error_wrong_argc)
	LOG_L(tr_args[0], "%p")
	LOG_L(tr_args[1], "%p")
	LOG_L(exe->r, "%p")
	switch (EXE_BIT(exe))
	{
	case 0:
		*tr_args[0] = *(uint8_t *)tr_args[1];
		return 0;
	case 1:
		*tr_args[0] = *(uint16_t *)tr_args[1];
		return 0;
	case 2:
		*tr_args[0] = *(uint32_t *)tr_args[1];
		return 0;
	case 3:
		*tr_args[0] = *(uint64_t *)tr_args[1];
		return 0;
	}
	printf("ERROR: it shouldn't be reached\n");
	while (1)
	{
	}
}

static int lang_dump(void *exe_, void *instr_)
{
	PREPARE_CAST
	executor_dump(stdout, exe, 0, 0);
	return 0;
}

static int lang_mode(void *exe_, void *instr_)
{
	PREPARE_CAST

	CHECK(instr->argc == 1, e_exe_error_wrong_argc)
	CHECK(instr->argv[0].content == e_arg_is_const, e_exe_error_wrong_arg_type)

	Word c = instr->argv[0].c;
	CHECK(c < 16 && !((c & e_exe_flag_word_float) &&
					  ((c & 3) < 2 || (c & e_exe_flag_word_signed) == 0)),
		  e_exe_error_wrong_const)

	exe->r[e_exe_reg_rf] |= e_exe_flags_word;
	exe->r[e_exe_reg_rf] ^= e_exe_flags_word;
	exe->r[e_exe_reg_rf] |= c;
	return 0;
}

static int lang_jmp(void *exe_, void *instr_)
{
	PREPARE_ARGS
	CHECK(instr->argc == 1, e_exe_error_wrong_argc)

	exe->r[e_exe_reg_rip] += instr->argv[0].c;
	return 0;
}

static int lang_call(void *exe_, void *instr_)
{
	PREPARE_ARGS
	CHECK(instr->argc == 1, e_exe_error_wrong_argc)

	Word rbp = exe->r[e_exe_reg_rbp];
	Word rsp = exe->r[e_exe_reg_rsp];
	Word rip = exe->r[e_exe_reg_rip];
	uint8_t *memory = exe->memory;
	*(Word *)(memory + rsp) = rbp;
	*(Word *)(memory + rsp + sizeof(Word)) = rip;
	exe->r[e_exe_reg_rbp] = rsp;
	exe->r[e_exe_reg_rsp] = rsp + 2 * sizeof(Word);
	exe->shift_reg += 4;

	exe->r[e_exe_reg_rip] += instr->argv[0].c;
	return 0;
}

static int lang_ret(void *exe_, void *instr_)
{
	PREPARE_ARGS
	CHECK(instr->argc == 0, e_exe_error_wrong_argc)

	Word rbp = exe->r[e_exe_reg_rbp];
	uint8_t *memory = exe->memory;

	exe->r[e_exe_reg_rbp] = *(Word *)(memory + rbp);
	exe->r[e_exe_reg_rip] = *(Word *)(memory + rbp + sizeof(Word));
	exe->r[e_exe_reg_rsp] = exe->r[e_exe_reg_rbp] + 2 * sizeof(Word);
	exe->shift_reg -= 4;
	return 0;
}

#define OPERATE(operator)                                         \
	LOG_L((exe->r[e_exe_reg_rf] & e_exe_flags_word), "%ld")       \
	LOG_L(tr_args[0], "%p")                                       \
	LOG_L(tr_args[1], "%p")                                       \
	LOG_L(*tr_args[0], "0x%lx")                                   \
	LOG_L(*tr_args[1], "0x%lx")                                   \
	switch (exe->r[e_exe_reg_rf] & e_exe_flags_word)              \
	{                                                             \
	case 0:                                                       \
		*(uint8_t *)tr_args[0] operator*(uint8_t *) tr_args[1];   \
		return 0;                                                 \
	case 1:                                                       \
		*(uint16_t *)tr_args[0] operator*(uint16_t *) tr_args[1]; \
		return 0;                                                 \
	case 2:                                                       \
		*(uint32_t *)tr_args[0] operator*(uint32_t *) tr_args[1]; \
		return 0;                                                 \
	case 3:                                                       \
		*(uint64_t *)tr_args[0] operator*(uint64_t *) tr_args[1]; \
		return 0;                                                 \
	case 4:                                                       \
		*(int8_t *)tr_args[0] operator*(int8_t *) tr_args[1];     \
		return 0;                                                 \
	case 5:                                                       \
		*(int16_t *)tr_args[0] operator*(int16_t *) tr_args[1];   \
		return 0;                                                 \
	case 6:                                                       \
		*(int32_t *)tr_args[0] operator*(int32_t *) tr_args[1];   \
		return 0;                                                 \
	case 7:                                                       \
		*(int64_t *)tr_args[0] operator*(int64_t *) tr_args[1];   \
		return 0;                                                 \
	case 14:                                                      \
		*(float *)tr_args[0] operator*(float *) tr_args[1];       \
		return 0;                                                 \
	case 15:                                                      \
		*(double *)tr_args[0] operator*(double *) tr_args[1];     \
		return 0;                                                 \
	default:                                                      \
		exe->error |= e_exe_error_wrong_word_type;                \
		return 1;                                                 \
	}

static int lang_add(void *exe_, void *instr_)
{
	PREPARE_ARGS
	OPERATE(+=)
}

static int lang_mul(void *exe_, void *instr_)
{
	PREPARE_ARGS
	OPERATE(*=)
}

#define MASTER_SUB(CASE, TYPE)                      \
	case CASE:                                      \
		*(TYPE *)tr_args[0] -= *(TYPE *)tr_args[1]; \
		return 0;
static int lang_sub(void *exe_, void *instr_)
{
	PREPARE_ARGS
	MASTER_CASE(MASTER_SUB)
}

#define MASTER_DIV(CASE, TYPE)                                     \
	case CASE:                                                     \
		CHECK(*(TYPE *)tr_args[1] != 0, e_exe_error_zero_division) \
		*(TYPE *)tr_args[0] /= *(TYPE *)tr_args[1];                \
		return 0;
static int lang_div(void *exe_, void *instr_)
{
	PREPARE_ARGS
	MASTER_CASE(MASTER_DIV)
}