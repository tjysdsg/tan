#!/bin/bash
cppcheck --enable=all -I./ -Iinclude/ -i cmake-build-debug . 2> cppcheck.txt

