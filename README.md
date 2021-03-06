`tan` programming language
===

**The project is being overhauled**

[![codecov](https://codecov.io/gh/tjysdsg/tan/branch/master/graph/badge.svg)](https://codecov.io/gh/tjysdsg/tan)
[![CodeFactor](https://www.codefactor.io/repository/github/tjysdsg/tan/badge)](https://www.codefactor.io/repository/github/tjysdsg/tan)
[![Code Inspector](https://www.code-inspector.com/project/8230/score/svg)](https://frontend.code-inspector.com/public/project/8230/tan/dashboard)

*Work In Progress*

# Design Objectives

- High performance
- Parallel programming
- Memory safety (but not as rigid as rust)
- Easy-to-use but powerful meta-programming
- Improved OOP model

# Build Instructions

## Prerequisites

- Make sure to clone this repository using `git clone --recursive`. Because there are some git submodules in [dep/](dep)
- [CMake](https://cmake.org/download) >=3.0, other versions might work, but not tested (and won't be tested)
- [LLVM-10 and Clang-10](https://releases.llvm.org/download.html), and their headers and libraries
- [libunwind](https://www.nongnu.org/libunwind/)

## Optional Dependencies

- Python and [gcovr](https://gcovr.com/en/stable/installation.html) for test coverage
- Doxygen for building documentations


Also, you can checkout this [Dockerfile](https://github.com/tjysdsg/tan/blob/docker/ubuntu/Dockerfile), used as the environment for the continuous integration of this project (based on Ubuntu-18.04)

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
- `ENABLE_COVERAGE`: bool Enable test coverage, default OFF
- `BUILD_EXAMPLES`: bool, Build `tan` examples, default OFF

As an example, the following command disable precompiled headers and enable test coverage:

```shell script
cmake .. -DENABLE_PRECOMPILED_HEADERS=OFF -DENABLE_COVERAGE=ON
```

### Note

It's recommended to use `clang` to compile the project, since it is much faster than `gcc`. Also, use `gold` as linker if possible to speed up linking time.

I am not sure what is causing the dramatic difference in the build time between `clang` and `gcc`, so I'm using LLVM toolchains for all current development.

# Project Structure

- cmake: CMake scripts and modules
- dep: third-party dependencies
- docs: documentations generated by Doxygen
- examples: example programs written in `tan`
- include: public headers
- runtime: `tan` runtime source files (both in tan and C++)
- scripts: several handy scripts
- src: source files
    - ast: Abstract Syntax Tree
    - base: basic utilities
    - cli: commandline interface of tanc compiler
    - compiler: main tanc logic
    - lexer: lexing/tokenization
    - lib: `libtan` sources
    - linker: linker implemented using Clang
    - parser: code related to parsing tokenized source into AST
    - test: test programs
        - test_src: tan sources used for general compiler tests
