#!/bin/bash
sudo apt-get -y install build-essential cmake
sudo apt-get -y remove llvm* clang* gcc* # remove older versions of llvm, clang, and gcc

# install llvm-9
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 9
sudo apt-get -y install libllvm-9-ocaml-dev libllvm9 llvm-9 llvm-9-dev llvm-9-doc llvm-9-examples llvm-9-runtime
sudo apt-get -y install clang-9 clang-tools-9 clang-9-doc libclang-common-9-dev libclang-9-dev libclang1-9 clang-format-9 python-clang-9 clangd-9
sudo apt-get -y install libfuzzer-9-dev
sudo apt-get -y install lldb-9
sudo apt-get -y install lld-9
sudo apt-get -y install libc++-9-dev libc++abi-9-dev
sudo apt-get -y install libomp-9-dev
sudo apt-get -y install libxml2 libxml2-dev

git submodule init
git submodule update --recursive
