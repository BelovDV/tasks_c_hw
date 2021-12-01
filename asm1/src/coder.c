#include "coder.h"
#include "log.h"

#include <string.h>

typedef unsigned char Byte;

#define CHECK_RETURN(condition) \
	{                           \
		if (!(condition))       \
		{                       \
			LOG_OUT             \
			return 0;           \
		}                       \
	}

#ifdef CODER_v_1
#else

enum
{
	b_error = e_lang_instr_count,
	b_data,
};

#if b_data > 255
#error thirst byte code overload
#endif

static size_t code_arg(Byte *dest, Argument *arg)
{
	dest[0] = arg->mem;
	dest[1] = arg->rx;
	dest[2] = arg->k;
	dest[3] = arg->ry;
	if (arg->c)
	{
		memcpy(dest + 4, &arg->c, sizeof(Word));
		return 12;
	}
	return 4;
}

size_t coder_code(void *dest_, Line *line)
{
	Byte *dest = dest_;
	LOG_IN
	LOG("%d", line->type)
	CHECK_RETURN(line->type == e_line_data || line->type == e_line_instr)
	if (line->type == e_line_data)
	{
		LOG("%lu", line->data.size)
		dest[0] = b_data;
		dest[1] = (Byte)line->data.size;
		dest += 2;
		memcpy(dest, line->data.data, line->data.size);
		LOG_OUT
		return line->data.size + 2;
	}
	LOG("%d", line->instr.argc)
	size_t sz = 2;
	dest[0] = line->instr.id;
	dest[1] = line->instr.argc;
	for (int arg = 0; arg < line->instr.argc; ++arg)
	{
		size_t arg_sz = code_arg(dest + sz, &line->instr.argv[arg]);
		CHECK_RETURN(arg_sz != 0)
		sz += arg_sz;
	}
	LOG_OUT
	return sz;
}

#endif