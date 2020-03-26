#!/bin/bash
mkdir -p fuzzing/tanc/
afl-fuzz -m 1000 -i src/test/test_src/ -o fuzzing/tanc/ bin/tanc_fuzzing @@
