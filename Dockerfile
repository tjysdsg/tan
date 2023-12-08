FROM ubuntu:20.04

RUN apt-get update

# disable command line interaction when installing tzdata
RUN export DEBIAN_FRONTEND=noninteractive
RUN ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
RUN apt-get install -y tzdata
RUN dpkg-reconfigure --frontend noninteractive tzdata

# common libraries and dependencies of llvm
RUN apt-get install -y lsb-release build-essential wget gpg software-properties-common git zlib1g zlib1g-dev curl libtinfo5 libtinfo-dev libxml2 libxml2-dev zip unzip

# install latest cmake from https://apt.kitware.com/
RUN apt -y remove cmake
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null
RUN apt-get update
RUN rm /usr/share/keyrings/kitware-archive-keyring.gpg
RUN apt-get -y install kitware-archive-keyring
RUN apt-get -y install cmake

# LLVM
RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 16

RUN apt-get -y install libllvm-16-ocaml-dev libllvm16 llvm-16 llvm-16-dev llvm-16-doc llvm-16-examples llvm-16-runtime
RUN apt-get -y install clang-16 clang-tools-16 clang-16-doc libclang-common-16-dev libclang-16-dev libclang1-16 clang-format-16 clangd-16 libclang-rt-16-dev
RUN apt-get -y install libfuzzer-16-dev lldb-16 lld-16 lld liblld-16-dev libc++-16-dev libc++abi-16-dev libomp-16-dev libunwind-16 libunwind-16-dev

# make sure `clang` and `clang++` are in PATH
RUN rm -f /usr/bin/clang /usr/bin/clang++; ln -s /usr/bin/clang-16 /usr/bin/clang; ln -s /usr/bin/clang++-16 /usr/bin/clang++
