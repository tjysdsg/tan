.PHONY: output.o print_shit.h print_shit.c
	g++ output.o print_shit.h print_shit.c -o a.out

output.o: main.tan
	../../bin/tanc main.tan output.o

run: .PHONY
	./a.out

