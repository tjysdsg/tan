#!/bin/bash
mkdir -p build
pushd build
cmake .. || exit 1
make -j4 tanc || exit 1
popd
