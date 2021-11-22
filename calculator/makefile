args = -g -Wall -Wextra -Werror -fsanitize=address,leak -D FALL_ON_ERROR -D LOG_FILE='"../temp/asm1/data/log"' -D EXTRA_LOG -I ../library/*/*.h
bin=../temp/calculator
data=../temp/calculator

all:
	mkdir -p ${bin}
	gcc -o ${bin}/Calculator main.c calculator.c ${args}
	bin/Calculator

clean:
	rm -fr bin
