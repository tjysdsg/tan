#!/bin/bash
sudo apt-get -y install build-essential cmake
sudo apt-get -y remove llvm* clang* # remove older versions of llvm and clang

# install llvm-9
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 9

git submodule init
git submodule update --recursive
