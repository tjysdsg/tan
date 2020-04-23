#!/bin/bash
export CC=$(which clang-9) || exit 1
export CXX=$(which clang++-9) || exit 1
llvm_prefix=$(llvm-config --includedir)
llvm_prefix=$(dirname ${llvm_prefix})
echo "LLVM prefix is: ${llvm_prefix}"

mkdir -p build
pushd build
cmake -DCMAKE_PREFIX_PATH=${llvm_prefix} .. || exit 1
make -j8 || exit 1
popd

./scripts/tanc_test.sh || exit 1

pushd src/test
../../bin/tan_tests || exit 1
popd
