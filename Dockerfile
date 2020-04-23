FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y lsb-release build-essential cmake wget software-properties-common
RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 9
RUN apt-get -y install libllvm-9-ocaml-dev libllvm9 llvm-9 llvm-9-dev llvm-9-doc llvm-9-examples llvm-9-runtime
RUN apt-get -y install clang-9 clang-tools-9 clang-9-doc libclang-common-9-dev libclang-9-dev libclang1-9 clang-format-9 python-clang-9 clangd-9
RUN apt-get -y install libfuzzer-9-dev
RUN apt-get -y install lldb-9
RUN apt-get -y install lld-9
RUN apt-get -y install libc++-9-dev libc++abi-9-dev
RUN apt-get -y install libomp-9-dev
RUN apt-get -y install libxml2 libxml2-dev
