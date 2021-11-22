all:
	@echo nothing

asm: mkdir
	gcc -o bin/assembler -I inc main/assembler.c src/*.c

disasm: mkdir
	gcc -o bin/disassembler -I inc main/disassembler.c src/*.c


exe: mkdir
	gcc -o bin/executor -I inc main/executor.c src/*.c

mkdir:
	mkdir -p bin

clean:
	rm -rf bin