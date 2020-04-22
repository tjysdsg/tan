#!/bin/bash
export CC=$(which clang)
export CXX=$(which clang++)
mkdir -p build
pushd build
cmake .. -DCMAKE_BUILD_TYPE=Release || exit 1
make -j8 || exit 1
popd

# artifact
mkdir -p tan-release
mkdir -p tan-release/include/tan
cp -r lib tan-release/
cp -r bin tan-release/
cp -r include/* tan-release/include/tan/
