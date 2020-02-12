mkdir -p build
pushd build
cmake ..
make -j*
popd
