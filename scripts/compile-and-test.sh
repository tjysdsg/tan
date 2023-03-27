#!/bin/bash
mkdir -p build
pushd build
export CC=clang-15
export CXX=clang++-15
cmake .. -DENABLE_COVERAGE=ON || exit 1
make -j4 tests || exit 1
popd
