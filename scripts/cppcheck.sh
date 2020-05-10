#!/bin/bash
cppcheck --enable=all -I./ -Iinclude/ include/ src/ --force 2>cppcheck.log
cat cppcheck.log
