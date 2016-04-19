#!/bin/sh -e
version=`git describe --tags`
tar -cjf measurement_kit-jni-$version.tar.bz2 jni/
