all: run

a.out:
	../../bin/tanc main.tan main.tan.o
	clang print_shit.h print_shit.c -o print_shit.o
	../../bin/tan-ld main.tan.o print_shit.o -o a.out

run: a.out
	./a.out

