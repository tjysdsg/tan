#!/bin/bash
mkdir -p build
pushd build
cmake .. -DENABLE_COVERAGE=ON || exit 1
make -j4 || exit 1
popd
