FROM ubuntu:18.04

RUN apt-get update

# Fucking tzdata asking for input
RUN export DEBIAN_FRONTEND=noninteractive
RUN ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
RUN apt-get install -y tzdata
RUN dpkg-reconfigure --frontend noninteractive tzdata

RUN apt-get install -y lsb-release build-essential cmake wget software-properties-common git zlib1g zlib1g-dev curl
RUN apt-get install -y libtinfo5 libtinfo-dev libxml2 libxml2-dev zip unzip gcovr
RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh

RUN ./llvm.sh 10

RUN apt-get -y install libllvm-10-ocaml-dev libllvm10 llvm-10 llvm-10-dev llvm-10-doc llvm-10-examples llvm-10-runtime
RUN apt-get -y install clang-10 clang-tools-10 clang-10-doc libclang-common-10-dev libclang-10-dev libclang1-10 clang-format-10 clangd-10
RUN apt-get -y install libfuzzer-10-dev
RUN apt-get -y install lldb-10
RUN apt-get -y install lld-10 lld
RUN apt-get -y install libc++-10-dev libc++abi-10-dev
RUN apt-get -y install libomp-10-dev
RUN apt-get -y install libunwind8 libunwind-dev

# Python for cpp-coveralls
RUN apt-get -y install python3 python3-pip
RUN pip3 install cpp-coveralls
