#include "language.h"
#include "log.h"
#include "check.h"

char language_keywords[e_lang_key_count][e_max_word_length] = {
	"___",
	"exit", "write_num", "read_num", "write_char", "read_char", "write_str", "read_str", "sqrt"};

#define LANG_FUNC_DECLARATION(name) int i_##name(Executor *executor);
LANG_DECLARATIONS(LANG_FUNC_DECLARATION)
#undef LANG_FUNC_DECLARATION

#define LANG_DESCRIPTION_CONTENT(name) #name,
char mnemonics[e_lang_instr_count][e_max_word_length] = {
	LANG_DECLARATIONS(LANG_DESCRIPTION_CONTENT)};
#undef LANG_DESCRIPTION_CONTENT

static int parse_arg(Executor *exe, int argn);

int language_execute(Executor *exe)
{
	debug_check(exe != NULL);
	for (int i = 0; i < exe->instr.instr.argc; ++i)
	{
		parse_arg(exe, i);
		LOG("%p", exe->v_ptr[i])
		LOG("%lu", exe->constant[i])
	}

#define LANG_SWITCH(name)     \
	case e_lang_instr_##name: \
		return i_##name(exe);
	switch (exe->instr.instr.id)
	{
		LANG_DECLARATIONS(LANG_SWITCH)
	default:
		return 1;
	}
#undef LANG_SWITCH
}

#define CHECK_RETURN(condition, message)            \
	{                                               \
		if (!(condition))                           \
		{                                           \
			exe->r[e_exe_reg_rf] |= e_exe_rf_error; \
			log_error(exe, message);                \
			return 1;                               \
		}                                           \
	}

#define FOR_MODE_SWITCH(FUNC)             \
	switch (exe->r[e_exe_reg_rf])         \
	{                                     \
	case 0:                               \
		FUNC(unsigned char)               \
		break;                            \
	case 1:                               \
		FUNC(unsigned short)              \
		break;                            \
	case 2:                               \
		FUNC(unsigned int)                \
		break;                            \
	case 3:                               \
		FUNC(unsigned long)               \
		break;                            \
	case 4:                               \
		FUNC(char)                        \
		break;                            \
	case 5:                               \
		FUNC(short)                       \
		break;                            \
	case 6:                               \
		FUNC(int)                         \
		break;                            \
	case 7:                               \
		FUNC(long)                        \
		break;                            \
	case 8:                               \
		FUNC(float)                       \
		break;                            \
	case 9:                               \
		FUNC(double)                      \
		break;                            \
	default:                              \
		CHECK_RETURN(0, "wrong num mode") \
	}

#define FOR_MODE_SWITCH_PRINT(FUNC)       \
	switch (exe->r[e_exe_reg_rf])         \
	{                                     \
	case 0:                               \
		FUNC("%c", unsigned char)         \
		break;                            \
	case 1:                               \
		FUNC("%hd", unsigned short)       \
		break;                            \
	case 2:                               \
		FUNC("%u", unsigned int)          \
		break;                            \
	case 3:                               \
		FUNC("%lu", unsigned long)        \
		break;                            \
	case 4:                               \
		FUNC("%c", char)                  \
		break;                            \
	case 5:                               \
		FUNC("%hd", short)                \
		break;                            \
	case 6:                               \
		FUNC("%d", int)                   \
		break;                            \
	case 7:                               \
		FUNC("%ld", long)                 \
		break;                            \
	case 8:                               \
		FUNC("%f", float)                 \
		break;                            \
	case 9:                               \
		FUNC("%lf", double)               \
		break;                            \
	default:                              \
		CHECK_RETURN(0, "wrong num mode") \
	}

#define R(number) (exe->r[(number + exe->shift) & 15])
#define RF (exe->r[e_exe_reg_rf])
#define RIP (exe->r[e_exe_reg_rip])
#define RSP (exe->r[e_exe_reg_rsp])
#define RBP (exe->r[e_exe_reg_rbp])

#define ARG(i) (*(exe->v_ptr[i]))

static int parse_arg(Executor *exe, int argn)
{
	LOG_IN
	Argument *arg = &exe->instr.instr.argv[argn];
	Word *c = &exe->constant[argn];
	Word **result = &exe->v_ptr[argn];

	LOG("%d", (int)arg->rx)
	LOG("%d", (int)arg->k)
	LOG("%d", (int)arg->ry)
	LOG("%lu", arg->c)

	*c = 0;
	if (arg->rx != e_exe_reg_count &&
		arg->k == 0 && arg->mem == 0 && arg->c == 0)
	{
		LOG("only rx%s", "")
		*result = (arg->rx < 16 ? &R(arg->rx) : &exe->r[(int)arg->rx]);
		LOG_OUT
		return 0;
	}
	if (arg->rx != e_exe_reg_count)
		*c += R(arg->rx);
	if (arg->k)
		*c += (Word)arg->k * (arg->ry >= 16 ? exe->r[(int)arg->ry] : R(arg->ry));
	if (arg->is_c)
		*c = arg->c;
	if (arg->mem)
		*result = (Word *)((char *)exe->memory + *c);
	else
		*result = c;
	LOG_OUT
	return 0;
}

static void log_error(Executor *exe, const char *message)
{
	fprintf(exe->log, "ERROR: %s\n", message);
}

int i_error(Executor *exe)
{
	log_error(exe, "instr error was encountered");
	RF |= e_exe_rf_error;
	return 0;
}
int i_nope(Executor *exe)
{
	(void)exe;
	return 0;
}
#define I_LIBCALL_WRITE_NUM(arg, type) \
	printf(arg, *(type *)&R(4));
#define I_LIBCALL_READ_NUM(arg, type) \
	scanf(arg, (type *)&R(4));
int i_libcall(Executor *exe)
{
	debug_check(&R(1) != NULL);
	LOG("%lu", ARG(0))
	switch (ARG(0))
	{
	case e_lang_key_write_num:
		FOR_MODE_SWITCH_PRINT(I_LIBCALL_WRITE_NUM)
		break;
	case e_lang_key_read_num:
		FOR_MODE_SWITCH_PRINT(I_LIBCALL_READ_NUM)
		break;
	case e_lang_key_write_str: // string storage defined(
	{
		char *mem = exe->memory;
		char sz = mem[RIP + R(4)];
		char *str = mem + RIP + R(4) + 1;
		printf("%*s", (int)sz, str);
		break;
	}
	default:
		CHECK_RETURN(0, "libcall: wrong keyword")
	}
	return 0;
}
int i_syscall(Executor *exe)
{
	switch (ARG(0))
	{
	case e_lang_key_exit:
		RF |= e_exe_rf_end;
		break;
	default:
		CHECK_RETURN(0, "syscall: wrong keyword")
	}
	return 0;
}
#define I_MOV(type) \
	*(type *)(&ARG(0)) = *(type *)(&ARG(1));
int i_mov(Executor *exe)
{
	FOR_MODE_SWITCH(I_MOV)
	return 0;
}
int i_dump(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_mode(Executor *exe)
{
	RF &= ~e_exe_rf_mask_mode;
	RF |= e_exe_rf_mask_mode & exe->instr.instr.argv[0].c;
	return 0;
}
#define I_SUB(type) \
	*(type *)(&ARG(0)) -= *(type *)(&ARG(1));
int i_sub(Executor *exe)
{
	FOR_MODE_SWITCH(I_SUB)
	return 0;
}
#define I_DIV(type) \
	*(type *)(&ARG(0)) /= *(type *)(&ARG(1));
int i_div(Executor *exe)
{
	FOR_MODE_SWITCH(I_DIV)
	return 0;
}
#define I_ADD(type) \
	*(type *)(&ARG(0)) += *(type *)(&ARG(1));
int i_add(Executor *exe)
{
	FOR_MODE_SWITCH(I_ADD)
	return 0;
}
#define I_MUL(type) \
	*(type *)(&ARG(0)) *= *(type *)(&ARG(1));
int i_mul(Executor *exe)
{
	FOR_MODE_SWITCH(I_MUL)
	return 0;
}
int i_jmp(Executor *exe)
{
	LOG("%ld", *(Word *)&ARG(1))
	RIP += ARG(1);
	return 0;
}
int i_call(Executor *exe)
{
	*(Word *)((char *)exe->memory + RSP) = RIP;
	RSP += sizeof(Word);
	exe->shift += 4;
	return i_jmp(exe);
}
int i_ret(Executor *exe)
{
	RSP -= sizeof(Word);
	Word offset = *(Word *)((char *)exe->memory + RSP);
	LOG("%ld", offset);
	RIP = offset;
	exe->shift -= 4;
	return 0;
}
int i_if(Executor *exe)
{
	(void)exe;
	return 0;
}
#define I_PUSH(type)                                         \
	*(type *)((char *)exe->memory + RSP) = *(type *)&ARG(0); \
	RSP += sizeof(Word);
int i_push(Executor *exe)
{
	FOR_MODE_SWITCH(I_PUSH)
	return 0;
}
#define I_POP(type)      \
	RSP -= sizeof(Word); \
	*(type *)&ARG(0) = *(type *)((char *)exe->memory + RSP);
int i_pop(Executor *exe)
{
	FOR_MODE_SWITCH(I_POP)
	return 0;
}
int i_ifz(Executor *exe)
{
	LOG("%lu", ARG(0))
	if (ARG(0) != 0)
		RF |= e_exe_rf_skip;
	LOG("%lu", RF)
	return 0;
}