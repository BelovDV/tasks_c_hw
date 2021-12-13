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
    sh_error,
    sh_continue,
    sh_data,
    sh_syscall,
    sh_libcall,
    sh_ret,

    sh_count
};

enum
{
    mem_is_mem = 0x1,
    mem_is_c = 0x2,
};

static size_t code_short(Byte *dest, Line *arg)
{
    if (arg->instr.id == e_lang_instr_ret)
    {
        dest[0] = sh_ret;
        return 1;
    }
    if (arg->instr.id == e_lang_instr_syscall)
    {
        dest[0] = sh_syscall;
        dest[1] = (Byte)arg->instr.argv[0].c;
        return 2;
    }
    if (arg->instr.id == e_lang_instr_libcall)
    {
        dest[0] = sh_libcall;
        dest[1] = (Byte)arg->instr.argv[0].c;
        return 2;
    }
    if (arg->instr.id == e_lang_instr_error)
    {
        dest[0] = sh_error;
        return 1;
    }
    return 0;
}

static size_t decode_short(Byte *source, Line *result)
{
    if (source[0] == sh_ret)
    {
        result->instr.argc = 0;
        result->instr.id = e_lang_instr_ret;
        return 1;
    }
    if (source[0] == sh_syscall)
    {
        result->instr.argc = 1;
        result->instr.argv[0].is_c = 1;
        result->instr.argv[0].rx = e_exe_reg_count;
        result->instr.id = e_lang_instr_syscall;
        result->instr.argv[0].c = (Word)source[1];
        return 2;
    }
    if (source[0] == sh_libcall)
    {
        result->instr.argc = 1;
        result->instr.argv[0].is_c = 1;
        result->instr.argv[0].rx = e_exe_reg_count;
        result->instr.id = e_lang_instr_libcall;
        result->instr.argv[0].c = (Word)source[1];
        LOG("%lu", (Word)source[1])
        LOG("%d", (int)source[1])
        return 2;
    }
    if (source[0] == sh_error)
    {
        result->instr.argc = 0;
        result->instr.id = e_lang_instr_error;
        return 1;
    }
    return 0;
}

#if b_data > 255
#error thirst byte code overload
#endif

static size_t code_arg(Byte *dest, Argument *arg)
{
    // LOG("%p", arg)
    // LOG("%d", (int)arg->mem)
    // LOG("%d", (int)arg->rx)
    // LOG("%d", (int)arg->k)
    // LOG("%d", (int)arg->ry)
    // LOG("%lu", arg->c)
    LOG("%d", (int)arg->is_c)
    dest[0] = 0;
    if (arg->mem)
        dest[0] |= mem_is_mem;
    dest[1] = arg->rx;
    dest[2] = arg->k;
    dest[3] = arg->ry;
    if (arg->is_c)
    {
        dest[0] |= mem_is_c;
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
        dest[0] = sh_data;
        dest[1] = (Byte)line->data.size;
        dest += 2;
        memcpy(dest, line->data.data, line->data.size);
        dest[line->data.size] = '\0';
        LOG_OUT
        return line->data.size + 3;
    }

    size_t check_short = code_short(dest, line);
    if (check_short)
    {
        LOG_OUT
        return check_short;
    }

    LOG("%d", line->instr.argc)
    size_t sz = 3;
    dest[0] = sh_continue;
    dest[1] = line->instr.id;
    dest[2] = line->instr.argc;
    for (int arg = 0; arg < line->instr.argc; ++arg)
    {
        size_t arg_sz = code_arg(dest + sz, &line->instr.argv[arg]);
        CHECK_RETURN(arg_sz != 0)
        sz += arg_sz;
    }
    LOG_OUT
    return sz;
}

size_t decode_arg(Byte *src, Argument *arg)
{
    arg->mem = src[0] & mem_is_mem;
    arg->is_c = src[0] & mem_is_c;
    arg->rx = src[1];
    arg->k = src[2];
    arg->ry = src[3];
    if (arg->is_c)
    {
        memcpy(&arg->c, src + 4, sizeof(Word));
        return 12;
    }
    else
        arg->c = 0;
    return 4;
}

size_t coder_decode(void *source, Line *result)
{
    memset(result, 0, sizeof(*result));
    Byte *src = source;
    LOG_IN
    CHECK_RETURN(src[0] <= sh_count)
    if (src[0] == sh_data)
    {
        result->type = e_line_data;
        result->data.size = src[1];
        LOG("%d", (int)result->data.size)
        memcpy(result->data.data, src + 2, src[1] + 1);
        result->data.data[src[1]] = '\0';
        LOG_OUT
        return (size_t)src[1] + 3;
    }

    if (src[0] != sh_continue)
    {
        size_t check_short = decode_short(source, result);
        result->type = e_line_instr;
        LOG_OUT
        return check_short;
    }

    result->type = e_line_instr;
    Instruction *instr = &result->instr;
    instr->id = src[1];
    instr->argc = src[2];
    LOG("%d", instr->argc)
    size_t sz = 3;
    for (int arg = 0; arg < instr->argc; ++arg)
    {
        size_t arg_sz = decode_arg(src + sz, &instr->argv[arg]);
        CHECK_RETURN(arg_sz != 0)
        sz += arg_sz;
    }
    LOG_OUT
    return sz;
}

void coder_modify_jump(void *position, Word offset)
{
    LOG_IN
    LOG("%ld", offset)
    offset -= 27; // sizeof(jmp)
    LOG("%ld", offset)
    memcpy((Byte *)position + 19, &offset, sizeof(Word)); // second argument
    LOG_OUT
}

#endif
