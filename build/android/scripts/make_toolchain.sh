#!/bin/sh -e

if [ $# -ne 3 ]; then
    echo "Usage: $0 NDK_DIR ARCH API" 1>&2
    echo "  NDK_DIR: path where NDK is installed" 1>&2
    echo "  ARCH: aarch64-linux-android arm-linux-androideabi mipsel-linux-android mips64el-linux-android x86 x86_64" 1>&2
    echo "  API: 1 ... 21" 1>&2
    echo "Example usage (on MacOS using brew):" 1>&2
    echo "  $0 /usr/local/Cellar/android-ndk/r10e aarch64-linux-android 21" 1>&2
    exit 1
fi

ROOTDIR=$(cd $(dirname $0)/.. && pwd -P)

NDK_DIR=$1
ARCH=$2
API=$3
BASEDIR=$ROOTDIR/toolchain

# XXX shortcut for armeabi-v7a
# According https://developer.android.com/ndk/guides/standalone_toolchain.html
# the proper solution is to override CFLAGS
if [ $ARCH = "arm-linux-androideabi-v7a" ]; then
    echo "WARNING: converting $ARCH to arm-linux-androideabi" 1>&2
    ARCH="arm-linux-androideabi"
fi

MAKE_TOOLCHAIN=${NDK_DIR}/build/tools/make-standalone-toolchain.sh

INSTALL_DIR=$BASEDIR/${ARCH}-${API}
echo "Creating toolchain for API ${API} and ARCH ${ARCH}-4.9 in ${INSTALL_DIR}"

# Bash is recommended to make the toolchain
bash $MAKE_TOOLCHAIN \
  --platform=android-${API} \
  --toolchain=${ARCH}-4.9 \
  --install-dir=${INSTALL_DIR} \
  --use-llvm \
  --stl=libc++

if [ $ARCH = x86 ]; then
    cp $NDK_DIR/sources/cxx-stl/llvm-libc++/libs/x86/libc++_static.a $INSTALL_DIR/sysroot/usr/lib/
elif [ $ARCH = x86_64 ]; then
    cp $NDK_DIR/sources/cxx-stl/llvm-libc++/libs/x86_64/libc++_static.a $INSTALL_DIR/sysroot/usr/lib/
elif [ $ARCH = aarch64-linux-android ]; then
    cp $NDK_DIR/sources/cxx-stl/llvm-libc++/libs/arm64-v8a/libc++_static.a $INSTALL_DIR/sysroot/usr/lib/
elif [ $ARCH = arm-linux-androideabi ]; then
    cp $NDK_DIR/sources/cxx-stl/llvm-libc++/libs/armeabi/libc++_static.a $INSTALL_DIR/sysroot/usr/lib/
elif [ $ARCH = mipsel-linux-android ]; then
    cp $NDK_DIR/sources/cxx-stl/llvm-libc++/libs/mips/libc++_static.a $INSTALL_DIR/sysroot/usr/lib/
elif [ $ARCH = mips64el-linux-android ]; then
    cp $NDK_DIR/sources/cxx-stl/llvm-libc++/libs/mips64/libc++_static.a $INSTALL_DIR/sysroot/usr/lib/
else
    echo "FATAL: unknown architecture: $ARCH" 1>&2
    exit 1
fi
