# CMake specific scripts

This directory contains CMake specific scripts. Namely:

- [autogen](autogen) is executed by `./autogen.sh --cmake` to generate the
  `./cmake/make.cmake` file containing all the targets to build.

- [run](run) runs CMake on your system. It contains a number of tricks to make
  sure CMake correctly finds dependencies in specific use cases (e.g. macOS with
  dependencies installed using [Homebew](https://brew.sh)).
