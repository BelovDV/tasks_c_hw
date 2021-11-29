#include "instruction.h"
#include "check.h"

#include <stdint.h>
#include <string.h>
#include <limits.h>

/*
enum E_argument
{
	// [rx+k*ry+c]
	e_arg_is_memory = 0x01,
	e_arg_is_rx = 0x02,
	e_arg_is_ry = 0x04,
	e_arg_is_const = 0x08
};
*/

enum E_compress
{
	e_memory = 0x01,
	e_rx = 0x02,
	e_ry = 0x04,
	e_log_c_1 = 0x08, // 00 - c=0; 01 - c=[]; 10 - c=[][][][]; 11 - c=uint64_t
	e_log_c_2 = 0x10,
	e_rx_1 = 0x20,
	e_rx_2 = 0x40,
	e_rx_3 = 0x80,

};

enum Lone
{
	id_error,
	id_next,
	id_nope,
	id_dump,
	id_syscall,
	id_libcall,
	id_mode,
	id_mov,
};

/*
[00......]
{
	[00000000]: error
	[00000001]: next
	[00000002]: nope // haven't done
	[00000003]: dump // haven't done
	[00000004]: syscall
	[00000005]: libcall
	[00000006]: mode
}
[01......]
{

}
[10......]
{

}
[11......] - mov short / jump
*/

typedef struct
{
	uint8_t mask; // flags and rx
	uint8_t ry;	  // [01......] - k; [.....567] - ry
	uint64_t c;
} Inner_arg;

typedef struct
{
	uint8_t id;
	Inner_arg args[e_const_max_arg_count];
} Inner_instr;

#define IX(argn) ((instr->argv[argn].rx) & 7)
#define IY(argn) ((instr->argv[argn].ry) & 7)
#define IK(argn) (instr->argv[argn].k)
#define IC(argn) (instr->argv[argn].c)
#define CONTENT(argn) (instr->argv[argn].content)
#define CHECK(condition) \
	if (!(condition))    \
	{                    \
		return 1;        \
	}

static size_t instr_code_arg(uint8_t *dest, Argument *arg)
{
	size_t length = 0;
	uint8_t *mask = dest;
	*mask = arg->content;
	LOG_L(*mask, "%hhx")
	if (*mask & e_arg_is_rx)
		*mask |= (arg->rx << 5);
	++length;
	LOG_L(arg->k, "%hhd")
	LOG_L(arg->rx, "%hhd")
	LOG_L(arg->ry, "%hhd")
	if (*mask & e_arg_is_ry)
	{
		if (arg->k == 1)
			arg->k = 0;
		else if (arg->k == 2)
			arg->k = 1;
		else if (arg->k == 4)
			arg->k = 2;
		else if (arg->k == 8)
			arg->k = 3;
		else
			return 0;
		dest[length++] = (arg->k << 6) + arg->ry;
	}
	if (!(*mask & e_arg_is_const))
		*mask &= ~(e_log_c_1 | e_log_c_2);
	else if (arg->c < (1lu << 8))
	{
		*mask |= e_log_c_1;
		*mask &= ~e_log_c_2;
		*(uint8_t *)(dest + length) = arg->c;
		length += 1;
	}
	else if (arg->c < (1lu << 32))
	{
		*mask |= e_log_c_2;
		*mask &= ~e_log_c_1;
		*(uint32_t *)(dest + length) = arg->c;
		length += 4;
	}
	else
	{
		*mask |= e_log_c_2 | e_log_c_1;
		*(uint64_t *)(dest + length) = arg->c;
		length += 8;
	}
	return length;
}

int instr_code(Instruction *instr, Array_frame *dest)
{
	if (dest->size + sizeof(Instruction) >= dest->capacity)
		utility_reserve(dest, (dest->size + sizeof(Instruction)) * 2);
	CHECK(instr->argc >= 0 && instr->argc <= e_const_max_arg_count);

	uint8_t *array = dest->array;
	array += dest->size;
	// jmp
	if (instr->id == e_lang_instr_jmp || instr->id == e_lang_instr_call)
	{
		LOG_I
		LOG_L(dest->size, "0x%lx")
		LOG_L(instr->argv[0].k, "%hhd")
		LOG_L(instr->argv[0].c, "0x%lx")
		if (instr->id == e_lang_instr_jmp)
		{
			array[0] = (3 << 6);
			LOG_L("", "jmp%s")
		}
		else
		{
			array[0] = (2 << 6);
			LOG_L("", "call%s")
		}
		int64_t change = IC(0);
		if (!IX(0))
			change -= dest->size;
		array += 1;
		dest->size += 1;
		if (IK(0) == -128)
		{
			if (change >= INT8_MIN && change <= INT8_MAX)
				IK(0) = 1;
			else if (change >= INT16_MIN && change <= INT16_MAX)
				IK(0) = 2;
			else if (change >= INT32_MIN && change <= INT32_MAX)
				IK(0) = 3;
			else if (change >= INT64_MIN && change <= INT64_MAX)
				IK(0) = 4;
			else
			{
				LOG_L(IC(0), "0x%lx")
				LOG_D
				return 1;
			}
		}
		LOG_L(change, "%ld")
		LOG_L(IX(0), "0x%x")
		if (!IX(0))
			change -= 1 + IK(0);
		LOG_L(change, "%ld")
		if (IK(0) == 1)
		{
			CHECK(change >= -128 && change < 128)
			array[0] = (int8_t)change;
		}
		else if (IK(0) == 2)
		{
			CHECK(change >= INT16_MIN && change <= INT16_MAX)
			*(int16_t *)array = (int16_t)change;
			array[-1] += 0x09;
		}
		else if (IK(0) == 4)
		{
			CHECK(change >= INT32_MIN && change <= INT32_MAX)
			*(int32_t *)array = (int32_t)change;
			array[-1] += 0x12;
		}
		else if (IK(0) == 8)
		{
			CHECK(change >= INT64_MIN && change <= INT64_MAX)
			*(int64_t *)array = (int64_t)change;
			array[-1] += 0x1b;
		}
		else
		{
			LOG_L(IK(0), "%hhd")
			LOG_D
			return 1;
		}
		dest->size += IK(0);
		LOG_L(dest->size, "0x%lx")
		LOG_D
		return 0;
	}
	// mov short
	if (instr->id == e_lang_instr_mov &&
		instr->argc == 2 &&
		CONTENT(0) == e_arg_is_rx &&
		CONTENT(1) == e_arg_is_rx)
	{
		LOG_L(IX(0), "%hhd")
		LOG_L(IX(1), "%hhd")
		CHECK(IX(0) != IX(1))
		array[0] = (3 << 6);
		array[0] += (IX(0) << 3);
		array[0] += IX(1);
		dest->size += 1;
		return 0;
	}
	// 1 short const arg
	if (
		instr->id == e_lang_instr_syscall ||
		instr->id == e_lang_instr_mode ||
		instr->id == e_lang_instr_libcall)
	{
		if (instr->id == e_lang_instr_syscall)
			array[0] = id_syscall;
		if (instr->id == e_lang_instr_libcall)
			array[0] = id_libcall;
		if (instr->id == e_lang_instr_mode)
			array[0] = id_mode;
		array[1] = instr->argv[0].c;
		dest->size += 2;
		return 0;
	}
	// 0 args
	if (
		instr->id == e_lang_instr_error ||
		instr->id == e_lang_instr_nope ||
		instr->id == e_lang_instr_dump)
	{
		if (instr->id == e_lang_instr_error)
			array[0] = id_error;
		if (instr->id == e_lang_instr_nope)
			array[0] = id_nope;
		if (instr->id == e_lang_instr_dump)
			array[0] = id_dump;
		dest->size += 1;
		return 0;
	}
	// mov
	/*
	if (instr->id == e_lang_instr_mov)
	{
		CHECK(instr->argc == 2)
	}
*/
	{
		array[0] = id_next;
		array[1] = instr->id;
		array[2] = instr->argc;
		size_t length = 3;
		for (int i = 0; i < instr->argc; ++i)
		{
			uint8_t *position = array + length;
			size_t delta = instr_code_arg(position, instr->argv + i);
			CHECK(delta)
			length += delta;
		}
		dest->size += length;
		return 0;
	}
}

size_t instr_decode(void *src_, Instruction *instr)
{
	uint8_t *src = src_;
	if ((src[0] >> 6) == 3 || (src[0] >> 6)) // mov short ; jump ; call
	{
		uint8_t rx = ((src[0] >> 3) & 7);
		uint8_t ry = (src[0] & 7);
		if (rx != ry)
		{
			instr->id = e_lang_instr_mov;
			instr->argc = 2;
			CONTENT(0) = e_arg_is_rx;
			CONTENT(1) = e_arg_is_rx;
			instr->argv[0].rx = rx;
			instr->argv[1].rx = ry;
			LOG_L(rx, "%hhu")
			LOG_L(ry, "%hhu")
			return 1;
		}
		else
		{
			LOG_L("", "jmp%s")
			LOG_L(rx, "%hhd")
			if ((src[0] >> 6) == 3)
				instr->id = e_lang_instr_jmp;
			else
				instr->id = e_lang_instr_call;
			instr->argc = 1;
			CONTENT(0) = e_arg_is_const;
			if (rx == 0)
			{
				IC(0) = (uint64_t)(int64_t) * (int8_t *)(src + 1);
				LOG_L(IC(0), "0x%lx")
				return 2;
			}
			if (rx == 1)
			{
				IC(0) = (uint64_t)(int64_t) * (int16_t *)(src + 1);
				LOG_L(IC(0), "0x%lx")
				return 3;
			}
			if (rx == 2)
			{
				IC(0) = (uint64_t)(int64_t) * (int32_t *)(src + 1);
				LOG_L(IC(0), "0x%lx")
				return 5;
			}
			if (rx == 3)
			{
				IC(0) = (uint64_t)(int64_t) * (int64_t *)(src + 1);
				LOG_L(IC(0), "0x%lx")
				return 9;
			}
			LOG_L(rx, "label error %d")
		}
		return 0;
	}
	else if ( // 1 short const arg
		src[0] == id_syscall ||
		src[0] == id_mode ||
		src[0] == id_libcall)
	{
		if (src[0] == id_syscall)
			instr->id = e_lang_instr_syscall;
		if (src[0] == id_libcall)
			instr->id = e_lang_instr_libcall;
		if (src[0] == id_mode)
			instr->id = e_lang_instr_mode;
		instr->argc = 1;
		IC(0) = src[1];
		CONTENT(0) = e_arg_is_const;
		return 2;
	}
	else if (
		src[0] == id_error ||
		src[0] == id_nope ||
		src[0] == id_dump)
	{
		if (src[0] == id_error)
			instr->id = e_lang_instr_error;
		if (src[0] == id_nope)
			instr->id = e_lang_instr_nope;
		if (src[0] == id_dump)
			instr->id = e_lang_instr_dump;
		CONTENT(0) = 0;
		return 1;
	}
	else if (src[0] == id_next)
	{
		instr->id = src[1];
		instr->argc = src[2];
		size_t length = 3;
		for (int i = 0; i < instr->argc; ++i)
		{
			uint8_t *mask = src + length;
			CONTENT(i) = *mask;
			LOG_L(instr->argv[i].content, "%hhd")
			LOG_L(*mask, "%hhx")
			if (*mask & e_arg_is_rx)
				instr->argv[i].rx = (*mask >> 5);
			++length;
			if (*mask & e_arg_is_ry)
			{
				IK(i) = (src[length] >> 6);
				instr->argv[i].ry = (src[length] & 7);
				++length;
			}
			CONTENT(i) |= e_arg_is_const;
			LOG_L(instr->argv[i].content, "%hhd")
			if ((*mask & e_log_c_1) && (*mask & e_log_c_2))
			{
				IC(i) = *(uint64_t *)(src + length);
				length += 8;
			}
			else if (*mask & e_log_c_2)
			{
				IC(i) = *(uint32_t *)(src + length);
				length += 4;
			}
			else if (*mask & e_log_c_1)
			{
				IC(i) = *(uint8_t *)(src + length);
				LOG_L(instr->argv[i].c, "%lu")
				length += 1;
			}
			else
				CONTENT(i) &= ~e_arg_is_const;
			LOG_L(instr->argv[i].content, "%hhd")
		}
		return length;
	}
	else
		return 0;
}

int instr_code_1_0(Instruction *instr, Array_frame *dest) // wasn't work
{
	if (dest->size + sizeof(Instruction) >= dest->capacity)
		utility_reserve(dest, (dest->size + sizeof(Instruction)) * 2);

	if (instr->id >= e_lang_instr_count)
		return 1;

	uint8_t *array = dest->array + dest->size;
	array[0] = e_lang_instr_count;
	array[1] = (uint8_t)instr->id;
	array[2] = instr->argc;
	array += 3;
	dest->size += 3;
	LOG_L(instr->argc, "%d")
	LOG_I
	for (int argn = 0; argn < instr->argc; ++argn)
	{
		Argument *arg = instr->argv + argn;
		array[0] = arg->content;
		array[1] = arg->rx;
		array[2] = arg->rx;
		array[3] = arg->k;
		array += 4;
		dest->size += 4;
		LOG_L(arg->content, "%x")
		if (arg->content & e_arg_is_const)
		{
			LOG_L(arg->c, "%lu")
			*(uint64_t *)array = arg->c; // little endian
			for (size_t step = 1; step <= 8; ++step)
				if (arg->c + 1 <= (1lu << (8 * step)))
				{
					array[-4] |= (uint8_t)(step << 4);
					array += step;
					dest->size += step;
					LOG_L(step, "%lu")
					break;
				}
		}
	}
	LOG_D
	return 0;
}

size_t instr_decode_1_0(void *src, Instruction *result) // wasn't work
{
	uint8_t *array = src;
	size_t shift = 0;
	if (array[0] != e_lang_instr_count)
		return 0;
	result->id = array[1];
	result->argc = array[2];
	if (result->id >= e_lang_instr_count)
		return 0;
	array += 3;
	shift += 3;
	for (int argn = 0; argn < result->argc; ++argn)
	{
		LOG_L(argn, "%d")
		Argument *arg = result->argv + argn;
		arg->content = array[0];
		arg->rx = array[1];
		arg->rx = array[2];
		arg->k = array[3];
		array += 4;
		shift += 4;
		uint8_t *iter = (uint8_t *)&arg->c;
		LOG_L(arg->content >> 4, "%d")
		for (size_t i = 1; i <= (arg->content >> 4); ++i)
			*(iter++) = *(array++), ++shift;
		LOG_L(arg->c, "%lu")
	}
	LOG_L(shift, "%lu")
	return shift;
}

void instr_code_v_0_1(Instruction *instruction, Array_frame *dest)
{
	if (dest->size + sizeof(Instruction) >= dest->capacity)
		utility_reserve(dest, (dest->size + sizeof(Instruction)) * 2);

	memcpy((uint8_t *)dest->array + dest->size, instruction, sizeof(Instruction) - sizeof(Argument));
	dest->size += sizeof(Instruction) - sizeof(Argument); // DEBUG - just smth starting)
}

size_t instr_decode_v_0_1(void *src, Instruction *result)
{
	memcpy(result, src, sizeof(Instruction) - sizeof(Argument));
	return sizeof(Instruction) - sizeof(Argument); // DEBUG - just smth starting)
}

void instr_arg_dump(FILE *stream, Argument *arg, const char *prefix)
{
	fprintf(
		stream,
		"%sarg dump:\n"
		"%s\tmemory: %d\n"
		"%s\trx:     %d %d\n"
		"%s\tk:      %d %d\n"
		"%s\try:     %d %d\n"
		"%s\tconst:  %d %lu\n",
		prefix,
		prefix, arg->content & e_arg_is_memory,
		prefix, (arg->content & e_arg_is_rx) / e_arg_is_rx, (int)arg->rx,
		prefix, (arg->content & e_arg_is_ry) / e_arg_is_ry, (int)arg->k,
		prefix, (arg->content & e_arg_is_ry) / e_arg_is_ry, (int)arg->ry,
		prefix, (arg->content & e_arg_is_const) / e_arg_is_const, arg->c);
}

void instr_dump(FILE *stream, Instruction *instr, const char *prefix)
{
	LOG_L(instr->id, "%d")
	fprintf(stream, "%sinstr %d %s\n", prefix, instr->id, language_instructions[instr->id].mnemonic);
	for (int i = 0; i < instr->argc; ++i)
		instr_arg_dump(stream, &instr->argv[i], prefix);
}