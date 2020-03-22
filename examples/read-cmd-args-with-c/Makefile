all: run

a.out:
	../../bin/tanc main.tan main.tan.o
	clang print_args.h print_args.c -o print_args.o
	../../bin/tan-ld main.tan.o print_args.o -o a.out

run: a.out
	./a.out arg1 arg2 arg3 arg4 arg5 arg6

