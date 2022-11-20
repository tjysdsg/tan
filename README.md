`tan` programming language
===

[![codecov](https://codecov.io/gh/tjysdsg/tan/branch/master/graph/badge.svg)](https://codecov.io/gh/tjysdsg/tan)

*Work In Progress*

# Build Instructions

## Prerequisites

- Make sure to clone this repository using `git clone --recursive`, because there are some git submodules
  under [dep/](dep)
- [CMake](https://cmake.org/download) >=3.0, other versions might work, but not tested (and won't be tested)
- [LLVM 10](https://releases.llvm.org/download.html). Only the libraries of LLVM and Clang are required. You can
  use [this](https://github.com/tjysdsg/llvm-build) precompiled version.
- `lld` is required to properly link the libraries (since it allows the libraries to be specified in arbitrary order
  which makes life MUCH easier linking against so many llvm libraries).
- gcc/clang compiler that supports C++ 17

- Others:

```bash
sudo apt install libz-dev libxml2-dev lzma-dev libpthread-stubs0-dev libtinfo5
```

## Optional Dependencies

- Python and [gcovr](https://gcovr.com/en/stable/installation.html) for test coverage
- Doxygen for building documentations

Also, you can check out this [Dockerfile](https://github.com/tjysdsg/tan/blob/docker/ubuntu/Dockerfile), used as the
environment for the continuous integration of this project (based on Ubuntu-18.04)

## Building

`tan` uses CMake as its build system.

To build the project, run the following command in project root directory

```shell script
mkdir -p build
cd build
cmake ..
make
```

The resulted binaries are in `bin` and `lib` directories.

### CMake options

There are a few options that you can specify when using `cmake` command:

- `ENABLE_PRECOMPILED_HEADERS`: bool, Enable precompiled headers to speed up build, default ON
- `ENABLE_COVERAGE`: bool, Enable test coverage, default OFF
- `BUILD_EXAMPLES`: bool, Build `tan` examples, default OFF
- `ENABLE_CCACHE`: bool, Enable ccache to speed up rebuilding, default ON
- `LLVM_ROOT_DIR`: Custom LLVM location
- `CLANG_ROOT_DIR`: Custom CLANG location

# Project Structure

- dep: third-party dependencies
- docs: documentations generated by Doxygen
- examples: example programs written in tan and C
- include: public headers
- runtime: `tan` runtime source files (both in tan and C++)
- scripts: several handy scripts
- src: source files
    - analysis: code analysis and type checking
    - ast: Abstract Syntax Tree
    - backtrace: wrapper around libbacktrace
    - base: basic functionalities
    - cli: commandline interface of tanc compiler
    - codegen: generate LLVM IR code
    - compiler: main tanc logic
    - lexer: lexing/tokenization
    - lib: `libtan` sources
    - linker: linker implemented using Clang
    - parser: code related to parsing the tokenized source into AST
- test:
    - test_exec: executable compilation tests
    - test_lib: library compilation tests
