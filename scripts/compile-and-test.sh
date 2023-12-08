#!/bin/bash
mkdir -p build
pushd build
export CC=clang-16
export CXX=clang++-16
cmake .. -DENABLE_COVERAGE=ON || exit 1
make -j4 coverage || exit 1
popd
