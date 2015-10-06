#!/bin/sh -e

ROOTDIR=$(cd $(dirname $(dirname $0)); pwd)

if [ $# -ne 2 ]; then
    echo "Usage: $0 ARCH API" 1>&2
    echo "  ARCH: aarch64-linux-android arm-linux-androideabi mipsel-linux-android mips64el-linux-android x86 x86_64" 1>&2
    echo "  API: 1 ... 21" 1>&2
    echo "Example usage (on MacOS using brew):" 1>&2
    echo " $0 aarch64-linux-android 21" 1>&2
    exit 1
fi

ARCH=$1
API=$2

export ANDROID_TOOLCHAIN=${ROOTDIR}/toolchain/${ARCH}-${API}
export SYSROOT=${ANDROID_TOOLCHAIN}/sysroot

# Note: the bin prefix (i686-linux-android) is different from the parameter
# that shall be passed to `make-standalone-toolchain.sh` (x86).
if [ $ARCH = x86 ]; then
    export TOOL_PATH=${ANDROID_TOOLCHAIN}/bin/i686-linux-android
elif [ $ARCH = x86_64 ]; then
    export TOOL_PATH=${ANDROID_TOOLCHAIN}/bin/x86_64-linux-android
    LIB_SUFFIX=64
elif [ $ARCH = "mips64el-linux-android" ]; then
    export TOOL_PATH=${ANDROID_TOOLCHAIN}/bin/${ARCH}
    LIB_SUFFIX=64
else
    export TOOL_PATH=${ANDROID_TOOLCHAIN}/bin/${ARCH}
fi

export CPP=${TOOL_PATH}-cpp
export AR=${TOOL_PATH}-ar
export AS=${TOOL_PATH}-as
export NM=${TOOL_PATH}-nm
export CC=${TOOL_PATH}-clang
export CXX=${TOOL_PATH}-clang++
export LD=${TOOL_PATH}-ld
export RANLIB=${TOOL_PATH}-ranlib
export STRIP=${TOOL_PATH}-strip

export CPPFLAGS="${CPPFLAGS} --sysroot=${SYSROOT} -I${SYSROOT}/usr/include -I${ANDROID_TOOLCHAIN}/include"
export LDFLAGS="${LDFLAGS} -L${SYSROOT}/usr/lib${LIB_SUFFIX} -L${ANDROID_TOOLCHAIN}/lib -lc++_static -latomic -lm"

(
    cd $ROOTDIR
    git submodule update --init --recursive  # Just in case
    install -d ${ROOTDIR}/build/${ARCH}-${API}
    cd ${ROOTDIR}/build/${ARCH}-${API}
    test -f Makefile && make clean
    echo "Configure with --host=${ARCH} and toolchain ${ANDROID_TOOLCHAIN}"
    test -x ${ROOTDIR}/measurement-kit/configure || (cd ${ROOTDIR}/measurement-kit/ && autoreconf -i)
    ${ROOTDIR}/measurement-kit/configure --host=${ARCH} --with-sysroot=${SYSROOT} \
      --with-libevent=builtin --with-libyaml-cpp=builtin --with-libboost=builtin \
      --disable-shared
    make V=0
    echo "Installing library in ${BASEDIR}/build/${ANDROID_TOOLCHAIN}"
    make install DESTDIR=${ROOTDIR}/dist/${ARCH}-${API}
)
