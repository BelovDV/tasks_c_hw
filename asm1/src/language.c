#include "language.h"

char language_keywords[e_lang_key_count][e_max_word_length] = {
	"exit", "write_num", "read_num", "write_char", "read_char", "write_str", "read_str", "sqrt"};

#define LANG_FUNC_DECLARATION(name) int i_##name(Executor *executor);
LANG_DECLARATIONS(LANG_FUNC_DECLARATION)
#undef LANG_FUNC_DECLARATION

#define LANG_DESCRIPTION_CONTENT(name) #name,
char language_mnemonics[e_lang_instr_count][e_max_word_length] = {
	LANG_DECLARATIONS(LANG_DESCRIPTION_CONTENT)};
#undef LANG_DESCRIPTION_CONTENT

int language_execute(Executor *exe)
{

#define LANG_SWITCH(name)     \
	case e_lang_instr_##name: \
		return i_##name(exe);
	switch (exe->instr.id)
	{
		LANG_DECLARATIONS(LANG_SWITCH)
	default:
		return 1;
	}
#undef LANG_SWITCH
}

int i_error(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_nope(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_libcall(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_syscall(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_mov(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_dump(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_mode(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_sub(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_div(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_add(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_mul(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_jmp(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_call(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_ret(Executor *exe)
{
	(void)exe;
	return 0;
}
int i_if(Executor *exe)
{
	(void)exe;
	return 0;
}