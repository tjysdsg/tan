mkdir -p gtest-build
mkdir -p dep

pushd dep
git clone https://github.com/google/googletest --depth=1
popd

pushd gtest-build
cmake ../dep/googletest/ -DGTEST_CREATE_SHARED_LIBRARY=1
make -j8
sudo make install
popd
