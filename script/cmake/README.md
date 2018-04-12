# CMake specific scripts

This directory contains CMake specific scripts. Namely:

- [autogen](autogen) updates files inside the `cmake/` directory to make sure
  the list of source files and of targets is in sync with the tree. This script
  is called by [./autogen.sh](../../autogen.sh).

- [run](run) runs CMake on your system. It contains a number of tricks to make
  sure CMake correctly finds dependencies in specific use cases (e.g. macOS with
  dependencies installed using [Homebew](https://brew.sh)).
