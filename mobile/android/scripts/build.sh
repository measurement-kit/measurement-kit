#!/bin/sh -e

ROOTDIR=$(cd $(dirname $(dirname $0)); pwd)

if [ $# -eq 3 ]; then
    NDK_DIR=$1
    ARCH=$2
    API=$3
elif [ $# -eq 2 ]; then
    NDK_DIR=$1
    API=$2
    for arch in aarch64-linux-android arm-linux-androideabi \
      arm-linux-androideabi-v7a mipsel-linux-android mips64el-linux-android  \
      x86 x86_64; do
        $0 $NDK_DIR $arch $API
    done
    exit 0
else
    echo "Usage: $0 NDK_DIR [ARCH] API" 1>&2
    echo "  NDK_DIR: path where NDK is installed" 1>&2
    echo "  ARCH: aarch64-linux-android arm-linux-androideabi mipsel-linux-android mips64el-linux-android x86 x86_64" 1>&2
    echo "  API: 1 ... 21" 1>&2
    echo "Example usage (on MacOS using brew):" 1>&2
    echo "  $0 /usr/local/Cellar/android-ndk/r10e/ x86 16" 1>&2
    echo "or" 1>&2
    echo "  $0 /usr/local/Cellar/android-ndk/r10e/ 16" 1>&2
    exit 1
fi

(
    cd $ROOTDIR
    if [ ! -d toolchain/${ARCH}-${API} ]; then
        ${ROOTDIR}/scripts/make_toolchain.sh ${NDK_DIR} ${ARCH} ${API}
    fi
    ${ROOTDIR}/scripts/build_target.sh ${ARCH} ${API}
)
