all: cache

cache: first.c
	gcc -Wall -Werror -fsanitize=address first.c -o first

clean:
	rm -rf cache
