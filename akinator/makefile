project = akinator

args = *.c -g -Wall -Wextra -fsanitize=address,leak
bin=../temp/akinator/bin
data=../temp/akinator/data

all: execute


compile: mkdir
	gcc -o ${bin}/akinator ${args}

execute: compile
	@echo
	@echo execute
	${bin}/akinator ${data}/txt

mkdir:
	mkdir -p ../temp
	mkdir -p ../temp/akinator
	mkdir -p ../temp/akinator/bin
	mkdir -p ../temp/akinator/data

clean:
	rm -rf ../temp/asm1
