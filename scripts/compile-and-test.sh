#!/bin/bash
export CC=$(which clang)
export CXX=$(which clang++)
mkdir -p build
pushd build
cmake .. || exit 1
make -j8 || exit 1
popd

./scripts/tanc_test.sh || exit 1

pushd src/test
../../bin/tan_tests || exit 1
popd
