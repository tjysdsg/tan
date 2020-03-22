all: a.out

a.out: main.tan.o
	../../bin/tan-ld $^ -o $@

main.tan.o:
	../../bin/tanc $^ $@
