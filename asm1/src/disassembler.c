#include "disassembler.h"
#include "language.h"
#include "coder.h"
#include "dull.h"
#include "log.h"
#include "check.h"
#include "utility.h"

typedef struct
{
	size_t *array;
	size_t size;
	size_t capacity;
} Labels;

typedef struct
{
	const Array_static_frame *bin;
	size_t start;
	size_t position;

	Line line;
	int section;
	Labels labels;

	FILE *dest;
	FILE *log;
} Disassembler;

static int decode(Disassembler *dis);
static int print_r(FILE *out, char r);			// return size of written
static int print_arg(FILE *out, Argument *arg); // return size of written
static int decode_instr(Disassembler *dis);
static int decode_data(Disassembler *dis, Line *line);
static int decode_jumps(Disassembler *dis);

#define SRC ((void *)((char *)dis->bin->array + dis->position))

int disassemble(const Array_static_frame *bin, FILE *dest, FILE *log)
{
	LOG_IN
	debug_check(dest != NULL && bin != NULL && log != NULL);
	fprintf(log, "Disassembler began working\n");
	Disassembler *dis = calloc(1, sizeof(Disassembler));
	Array_static_frame inner = {bin->array, bin->size};
	inner.array = (char *)(bin->array) + sizeof(struct Dull);
	inner.size -= sizeof(struct Dull);
	dis->bin = &inner;
	dis->dest = dest;
	dis->log = log;
	dis->section = e_section_none;
	LOG("%lu", dis->bin->size)
	dis->start = ((struct Dull *)bin->array)->start_offset;
	int is_error = decode(dis);
	if (is_error)
		fprintf(log, "Errors were encountered\n");
	fprintf(log, "\nDisassembler ended working\n");
	free(dis->labels.array);
	free(dis);
	LOG_OUT
	return is_error;
}

static int decode(Disassembler *dis)
{
	LOG_IN
	int is_error = 0;
	while (dis->position < dis->bin->size)
		is_error |= decode_jumps(dis);
	if (is_error)
	{
		LOG_OUT
		return 1;
	}
	dis->position = 0;
	while (dis->position < dis->bin->size)
	{
		for (size_t i = 0; i < dis->labels.size; ++i)
			if (dis->labels.array[i] == dis->position)
			{
				fprintf(dis->dest, ":0x%lx\n", dis->position);
				break;
			}
		if (dis->position == dis->start)
			fprintf(dis->dest, "_start\n");
		is_error |= decode_instr(dis);
		if (is_error)
		{
			fprintf(dis->log, "Disassembler failed\n");
			break;
		}
	}

	LOG_OUT
	return is_error;
}

static int print_r(FILE *out, char r)
{
	int vsp;
	if (r <= e_exe_reg_r15)
		fprintf(out, "r%d%n", (int)r, &vsp);
	else if (r == e_exe_reg_rbp)
		fprintf(out, "rbp%n", &vsp);
	else if (r == e_exe_reg_rsp)
		fprintf(out, "rsp%n", &vsp);
	else if (r == e_exe_reg_rip)
		fprintf(out, "rip%n", &vsp);
	else if (r == e_exe_reg_rf)
		fprintf(out, "rf%n", &vsp);
	else if (r == e_exe_reg_count)
		return 1; // todo...
	else
		vsp = 0;
	LOG("print_r %d", vsp)
	return vsp;
}

static int print_arg(FILE *out, Argument *arg)
{
	LOG_IN
	// LOG("%d", (int)arg->rx)
	// LOG("%d", (int)arg->k)
	// LOG("%d", (int)arg->ry)
	// LOG("%lu", arg->c)
	int size = 0;
	int vsp;
	if (arg->mem)
	{
		fprintf(out, "[");
		++size;
	}
	if (arg->rx != e_exe_reg_count)
	{
		vsp = print_r(out, arg->rx);
		size += vsp;
		if (!vsp)
		{
			LOG_OUT
			return 0;
		}
	}
	if (arg->k)
	{
		if (arg->rx != e_exe_reg_count)
		{
			fprintf(out, "+");
			++size;
		}
		fprintf(out, "%d*%n", arg->k, &vsp);
		size += vsp;
		vsp = print_r(out, arg->ry);
		size += vsp;
		if (!vsp)
		{
			LOG_OUT
			return 0;
		}
	}
	LOG("%lu", arg->c)
	if (arg->is_c)
	{
		if (arg->rx != e_exe_reg_count || arg->k)
		{
			LOG("%d", (int)arg->rx)
			LOG("%d", (int)arg->k)
			fprintf(out, "+");
			++size;
		}
		fprintf(out, "%lu%n", arg->c, &vsp);
		size += vsp;
	}
	if (arg->mem)
	{
		fprintf(out, "]");
		++size;
	}
	fprintf(out, " ");
	LOG_OUT
	return size + 1;
}

static int decode_instr(Disassembler *dis)
{
	size_t sz = coder_decode(SRC, &dis->line);
	LOG("%lu", sz)
	if (sz == 0)
		return 1;
	if (dis->line.type == e_line_data)
	{
		if (dis->section != e_section_data)
			fprintf(dis->dest, ".DATA\n");
		dis->section = e_section_data;
		int res = decode_data(dis, &dis->line);
		dis->position += sz;
		LOG("%d", res)
		return res;
	}

	if (dis->section != e_section_text)
		fprintf(dis->dest, ".TEXT\n");
	dis->section = e_section_text;
	Instruction *instr = &dis->line.instr;
	int len;
	LOG("%u", (unsigned)instr->id)
	fprintf(dis->dest, "\t%s %n", mnemonics[instr->id], &len);
	if (instr->id == e_lang_instr_jmp || instr->id == e_lang_instr_call)
	{
		len += 2;
		fprintf(dis->dest, "%lu ", instr->argv[0].c);
		int vsp;
		fprintf(dis->dest, "0x%lx %n",
				instr->argv[1].c + dis->position + sz, &vsp);
		len += vsp;
	}
	else
	{
		for (int i = 0; i < (int)instr->argc; ++i)
		{
			int vsp = print_arg(dis->dest, &instr->argv[i]);
			LOG("%d", vsp)
			if (!vsp)
				return 1;
			len += vsp;
		}
	}
	while (len++ < e_max_line_length)
		fprintf(dis->dest, " ");
	fprintf(dis->dest, " # 0x ");
	print_raw_data(dis->dest, SRC, sz);
	fprintf(dis->dest, "\n");
	dis->position += sz;
	return 0;
}

static int decode_data(Disassembler *dis, Line *line)
{
	LOG_IN;
	fprintf(dis->dest, "\tstring ");
	int vsp;
	fprintf(dis->dest, "%*.*s%n",
			(int)line->data.size,
			(int)line->data.size,
			line->data.data, &vsp);
	while (vsp++ < e_max_line_length - (int)sizeof("\tstring "))
		fprintf(dis->dest, " ");
	fprintf(dis->dest, " # 0x ");
	fwrite(line->data.data, line->data.size, 1, dis->dest);
	fprintf(dis->dest, "\n");
	LOG_OUT;
	return 0;
}

static int decode_jumps(Disassembler *dis)
{
	size_t sz = coder_decode(SRC, &dis->line);
	if (!sz)
		return 1;
	if (dis->line.type == e_line_instr &&
		(dis->line.instr.id == e_lang_instr_jmp ||
		 dis->line.instr.id == e_lang_instr_call))
	{
		utility_array_provide_space(&dis->labels, sizeof(size_t));
		dis->labels.array[dis->labels.size] =
			dis->position + dis->line.instr.argv[1].c + sz;
		LOG("0x%lx", dis->position)
		LOG("0x%lx", dis->labels.array[dis->labels.size])
		dis->labels.size += 1;
	}
	dis->position += sz;
	return 0;
}