mkdir -p build
pushd build
cmake .. || exit 1
if make -j8;
then
    pushd ../src/test
    ../../build/tan_tests
    popd
fi
popd
