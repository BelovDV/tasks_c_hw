project = asm1
program = factorial

args = src/*.c -g -Wall -Wextra -fsanitize=address,leak -I inc -D FALL_ON_ERROR -D LOG_FILE='"../temp/asm1/data/log"' -D EXTRA_LOG -rdynamic -D SHUTDOWN_REGEX -lm
# lm - temporal...
bin=../temp/asm1/bin
data=../temp/asm1/data

all: clear asm disasm exe run_asm run_disasm run_disasm_asm run_disasm_disasm run_disasm_exe

asm: mkdir
	gcc -o ${bin}/assembler main/assembler.c ${args}

disasm: mkdir
	gcc -o ${bin}/disassembler main/disassembler.c ${args}

exe: mkdir
	gcc -o ${bin}/executor main/executor.c ${args}

run_asm:
	@echo
	@echo run_asm
	rm -f ${bin}/${program}.dull
	${bin}/assembler examples/${program}.asm1 ${bin}/${program}.dull ${data}/log_assembler

run_disasm_asm:
	@echo
	@echo run_disasm_asm
	rm -f ${bin}/${program}d.dull
	${bin}/assembler ${data}/${program}d.asm1 ${bin}/${program}d.dull ${data}/log_assembler_d

run_disasm:
	@echo
	@echo run_disasm
	${bin}/disassembler ${bin}/${program}.dull ${data}/${program}d.asm1 ${data}/log_disassembler

run_disasm_disasm:
	@echo
	@echo run_disasm
	${bin}/disassembler ${bin}/${program}d.dull ${data}/${program}dd.asm1 ${data}/log_disassembler_d

run_exe:
	@echo
	@echo exe
	${bin}/executor ${bin}/${program}.dull ${data}/log_executor

run_disasm_exe:
	@echo
	@echo exe
	${bin}/executor ${bin}/${program}d.dull ${data}/log_executor_d

mkdir:
	mkdir -p ../temp/asm1
	mkdir -p ../temp/asm1/bin
	mkdir -p ../temp/asm1/data

test: clear
	@echo
	@echo test
	gcc -o ${bin}/test main/test.c src/*.c -g -Wall -Wextra -fsanitize=address,leak -I inc -D FALL_ON_ERROR -D LOG_FILE='"../temp/asm1/data/log"' -D EXTRA_LOG -rdynamic
	${bin}/test

clear:
	clear
	rm -f ../temp/asm1/data/log

clean:
	rm -rf ../temp/asm1