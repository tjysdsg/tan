mkdir -p build
pushd build
cmake ..
make -j4
if [ $? -eq 0 ]
then
    ctest -V
fi
popd
