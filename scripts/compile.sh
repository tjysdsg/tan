#!/bin/bash
mkdir -p build
pushd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo || exit 1
echo "Building target: $1"
make $1 || exit 1
popd
