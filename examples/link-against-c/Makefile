.PHONY: main.tan print_shit.h print_shit.c
	../../bin/tanc main.tan main.tan.o
	g++ -c print_shit.h print_shit.c -o print_shit.o
	../../bin/tan-ld main.tan.o print_shit.o -o a.out

run: .PHONY
	./a.out

all: .PHONY
