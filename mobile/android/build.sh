#!/bin/sh -e

ROOTDIR=$(cd $(dirname $(dirname $0)); pwd)

if [ $# -ne 3 ]; then
    echo "Usage: $0 NDK_DIR ARCH API" 1>&2
    echo "  NDK_DIR: path where NDK is installed" 1>&2
    echo "  ARCH: arm-linux-androideabi mipsel-linux-android x86" 1>&2
    echo "  API: 1 ... 21" 1>&2
    echo "Example usage (on MacOS using brew):" 1>&2
    echo "  $0 /usr/local/Cellar/android-ndk/r10e/ x86 16" 1>&2
    exit 1
fi

NDK_DIR=$1
ARCH=$2
API=$3

(
    cd $ROOTDIR
    if [ ! -d toolchain/${ARCH}-${API} ]; then
        ./scripts/make_toolchain.sh ${NDK_DIR} ${ARCH} ${API}
    fi
    ./scripts/build_target.sh ${ARCH} ${API}
)
