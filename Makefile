GCC = gcc
FLAGS = -Wextra -Wall -pedantic -lm
NAME = main
LIBNAME = libjson

INCLUDES = ./src/include/
LIB_ORIGIN = ./src/libs/

all: main.o dynamic_string.o binary_search_tree.o json_value.o arena.o
	${GCC} ${FLAGS} $^ -o ${NAME} -L ${LIB_ORIGIN}

lib: dynamic_string.o binary_search_tree.o json_value.o arena.o
	ar rcs ${LIBNAME}.a $^ -L ${LIB_ORIGIN}

main.o: main.c
	${GCC} ${FLAGS} -c $< -o $@ -L ${LIB_ORIGIN}

dynamic_string.o: ${LIB_ORIGIN}dynamic_string.c
	${GCC} ${FLAGS} -c $< -o $@ -L ${LIB_ORIGIN}

binary_search_tree.o: ${LIB_ORIGIN}binary_search_tree.c
	${GCC} ${FLAGS} -c $< -o $@ -L ${LIB_ORIGIN}

json_value.o: ${LIB_ORIGIN}json_value.c
	${GCC} ${FLAGS} -c $< -o $@ -L ${LIB_ORIGIN}

arena.o: ${LIB_ORIGIN}arena.c
	${GCC} ${FLAGS} -c $< -o $@ -L ${LIB_ORIGIN}

clear:
	rm *.o
	rm ${NAME}
