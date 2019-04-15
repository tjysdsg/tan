mkdir -p build
pushd build
cmake ..
if make -j4;
then
    pushd ../src/test
    ../../build/tan_tests
    popd
fi
popd
