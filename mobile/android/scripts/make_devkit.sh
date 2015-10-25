#!/bin/sh -e

if [ $# -ne 1 ]; then
    echo "Usage: $0 NDK_DIR" 1>&2
    echo "  NDK_DIR: path where NDK is installed" 1>&2
    echo "Example usage (on MacOS using brew):" 1>&2
    echo "  $0 /usr/local/Cellar/android-ndk/r10e" 1>&2
    exit 1
fi

$1/ndk-build
cp -R libs jniLibs
tar -cvzf measurement-kit-android-0.0.tar.gz java jniLibs
rm -rf jniLibs
