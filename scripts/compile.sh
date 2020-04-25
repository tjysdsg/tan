#!/bin/bash
mkdir -p build
pushd build
cmake -DCMAKE_PREFIX_PATH=${llvm_prefix} --target=$1 .. || exit 1
make || exit 1
popd
