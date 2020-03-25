all: run

a.out:
	../../bin/tanc main.tan
	clang -c print_args.c
	../../bin/tan-ld main.tan.o print_args.o -o a.out

run: a.out
	./a.out arg1 arg2 arg3 arg4 arg5 arg6

