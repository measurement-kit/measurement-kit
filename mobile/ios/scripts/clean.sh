#!/bin/sh -e

(
    cd measurement-kit
    test -f Makefile && make distclean
    git clean -dfx
)

rm -rf build/*
