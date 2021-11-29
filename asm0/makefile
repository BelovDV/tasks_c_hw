program=
args = src/*.c -g -Wall -Wextra -fsanitize=address,leak -I inc -D FALL_ON_ERROR -D UTILITY_LOG_FILE='"../temp/asm0/data/log_utility"' -D EXTRA_LOG -lm
# lm - temporal...
bin=../temp/asm0/bin
data=../temp/asm0/data

all: bash_clear asm disasm exe run_asm run_disasm run_exe

from_disasm: bash_clear asm disasm exe run_asm run_disasm run_disasm_asm run_exe

asm: mkdir
	gcc -o ${bin}/assembler main/assembler.c ${args}

disasm: mkdir
	gcc -o ${bin}/disassembler main/disassembler.c ${args}


exe: mkdir
	gcc -o ${bin}/executor main/executor.c ${args}

run_asm:
	@echo
	@echo run_asm
	rm -f ${data}/${program}.dull
	${bin}/assembler examples/${program}.asm0 ${data}/${program}.dull ${data}/log_assembler

run_disasm_asm:
	@echo
	@echo run_disasm_asm
	rm -f ${data}/${program}.dull
	${bin}/assembler ${data}/${program}d.asm0 ${data}/${program}.dull ${data}/log_assembler

run_disasm:
	@echo
	@echo run_disasm
	${bin}/disassembler ${data}/${program}.dull ${data}/${program}d.asm0 ${data}/log_disassembler

run_exe:
	@echo
	@echo exe
	${bin}/executor ${data}/${program}.dull ${data}/log_executor

vsp_gdb:
	@echo gdbserver localhost:8888 bin/assembler examples/1.asm0 examples/1.dull examples/log

mkdir:
	mkdir -p ../temp/asm0
	mkdir -p ../temp/asm0/bin
	mkdir -p ../temp/asm0/data

bash_clear:
	clear

clean:
	rm -rf bin