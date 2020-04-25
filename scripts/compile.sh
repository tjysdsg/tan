#!/bin/bash
mkdir -p build
pushd build
cmake -DCMAKE_PREFIX_PATH=${llvm_prefix} .. || exit 1
make || exit 1
popd
