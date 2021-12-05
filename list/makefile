args = *.c -g -Wall -Wextra -fsanitize=address,leak -O0

all: bash_clear list run
	dot -Tpng ../temp/list/graph -o ../temp/list/graph.png

list: mkdir
	gcc -o ../temp/list/list ${args}

run:
	../temp/list/list

mkdir:
	mkdir -p ../temp/list

bash_clear:
	clear

clean:
	rm -rf ../temp/list