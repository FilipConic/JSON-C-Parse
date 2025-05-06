GCC = gcc
FLAGS = -Wextra -Wall -pedantic -lm
NAME = main
LIBNAME = libjson

all: main.o dynamic_string.o binary_search_tree.o json_value.o arena.o
	${GCC} ${FLAGS} $^ -o ${NAME}

lib: dynamic_string.o binary_search_tree.o json_value.o arena.o
	ar rcs ${LIBNAME}.a $^

main.o: main.c
	${GCC} ${FLAGS} -c $< -o $@

dynamic_string.o: dynamic_string.c
	${GCC} ${FLAGS} -c $< -o $@

binary_search_tree.o: binary_search_tree.c
	${GCC} ${FLAGS} -c $< -o $@

json_value.o: json_value.c
	${GCC} ${FLAGS} -c $< -o $@

arena.o: arena.c
	${GCC} ${FLAGS} -c $< -o $@

clear:
	rm *.o
	rm ${NAME}
