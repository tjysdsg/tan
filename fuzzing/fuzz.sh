#!/usr/bin/env bash

# prepare test files
mkdir -p test_input
cp -r ../src/test/test_src/* test_input/

# build
export AFL_USE_UBSAN=1
export AFL_USE_ASAN=1
mkdir -p build
pushd build

export CC=$(which afl-clang-fast)
export CXX=$(which afl-clang-fast++)
export LD=$(which afl-clang-fast)
cmake ../.. -DLIB_OUTPUT_DIR=./lib -DEXE_OUTPUT_DIR=./bin -DENABLE_CCACHE=OFF
make -j6 tanc

popd

# fuzz!
afl-fuzz -i test_input/ -o out -m none -f test_case.tan -- ./build/bin/tanc test_case.tan -I.. -lruntime -L../runtime -o a.out

unset AFL_USE_UBSAN
unset AFL_USE_ASAN
unset CC
unset CXX
unset LD
