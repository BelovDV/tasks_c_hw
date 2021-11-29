project = asm1
program = ###

args = src/*.c -g -Wall -Wextra -fsanitize=address,leak -I inc -D FALL_ON_ERROR -D LOG_FILE='"../temp/asm1/data/log_utility"' -D EXTRA_LOG -lm
# lm - temporal...
bin=../temp/asm1/bin
data=../temp/asm1/data

all: bash_clear asm disasm exe run_asm run_disasm run_disasm_asm run_disasm_exe

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
	${bin}/assembler examples/${program}.asm1 ${data}/${program}.dull ${data}/log_assembler

run_disasm_asm:
	@echo
	@echo run_disasm_asm
	rm -f ${data}/${program}.dull
	${bin}/assembler ${data}/${program}d.asm1 ${data}/${program}.dull ${data}/log_assembler

run_disasm:
	@echo
	@echo run_disasm
	${bin}/disassembler ${data}/${program}.dull ${data}/${program}d.asm1 ${data}/log_disassembler

run_exe:
	@echo
	@echo exe
	${bin}/executor ${data}/${program}.dull ${data}/log_executor

mkdir:
	mkdir -p ../temp/asm1
	mkdir -p ../temp/asm1/bin
	mkdir -p ../temp/asm1/data

bash_clear:
	clear

clean:
	rm -rf ../temp/asm1