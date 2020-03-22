#!/bin/bash
sudo apt-get -y install build-essential cmake
sudo apt-get -y remove clang* # remove older versions of clang
# LLVM
REPO_NAME="deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-9 main"
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo add-apt-repository "${REPO_NAME}"
sudo apt-get update
sudo apt-get install -y clang-9 lldb-9 lld-9 clangd-9
sudo apt-get install -y libllvm-9-ocaml-dev libllvm9 llvm-9 llvm-9-dev llvm-9-doc llvm-9-examples llvm-9-runtime \
clang-9 clang-tools-9 clang-9-doc libclang-common-9-dev libclang-9-dev libclang1-9 clang-format-9 python-clang-9 \
clangd-9 libfuzzer-9-dev lldb-9 lld-9 libc++-9-dev libc++abi-9-dev libomp-9-dev

git submodule init
git submodule update --recursive
