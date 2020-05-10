#!/bin/bash
g++ -g -c print_args.h print_args.c -o print_args.o
../../bin/tanc main.tan main.tan.o
../../bin/tan-ld -o read-cmd-args main.tan.o print_args.o
