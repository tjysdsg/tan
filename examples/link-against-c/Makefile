all: run

a.out:
	../../bin/tanc main.tan
	clang -c print_shit.c
	../../bin/tan-ld main.tan.o print_shit.o -o a.out

run: a.out
	./a.out

