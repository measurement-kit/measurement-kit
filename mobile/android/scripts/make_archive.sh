#!/bin/sh -e
version=`git describe --tags`
tar -cjf measurement-kit-jni-$version.tar.bz2 jni/
