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
RUN ./llvm.sh 15

RUN apt-get -y install libllvm-15-ocaml-dev libllvm15 llvm-15 llvm-15-dev llvm-15-doc llvm-15-examples llvm-15-runtime
RUN apt-get -y install clang-15 clang-tools-15 clang-15-doc libclang-common-15-dev libclang-15-dev libclang1-15 clang-format-15 clangd-15 libclang-rt-15-dev
RUN apt-get -y install libfuzzer-15-dev lldb-15 lld-15 lld liblld-15-dev libc++-15-dev libc++abi-15-dev libomp-15-dev libunwind-15 libunwind-15-dev
