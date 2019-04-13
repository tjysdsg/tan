mkdir -p build
pushd build
cmake ..
make -j4
if [ $? -eq 0 ]
then
    pushd ../src/test
    ../../build/tan_tests
    popd
fi
popd
