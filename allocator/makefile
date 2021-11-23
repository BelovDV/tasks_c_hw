all:
	mkdir -p bin
	gcc -o Allocator *.c -g -Wall -Wextra -fsanitize=address,leak
	./Allocator

clean:
	rm -fr bin