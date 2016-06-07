#!/bin/sh -e

ROOTDIR=$(cd $(dirname $(dirname $0)) && pwd -P)
if [ $? -ne 0 ]; then
    echo "$0: cannot determine root directory" 1>&2
    exit 1
fi


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

# XXX shortcut for armeabi-v7a
# According https://developer.android.com/ndk/guides/standalone_toolchain.html
# the proper solution is to override CFLAGS
if [ $ARCH = "arm-linux-androideabi-v7a" ]; then
    echo "WARNING: converting $ARCH to arm-linux-androideabi" 1>&2
    ARCH="arm-linux-androideabi"
    V7A="-v7a"
else
    V7A=""
fi

export ANDROID_TOOLCHAIN=${ROOTDIR}/toolchain/${ARCH}-${API}
export SYSROOT=${ANDROID_TOOLCHAIN}/sysroot

# Note: the bin prefix (i686-linux-android) is different from the parameter
# that shall be passed to `make-standalone-toolchain.sh` (x86).
if [ $ARCH = aarch64-linux-android ]; then
    export TOOL_PATH=${ANDROID_TOOLCHAIN}/bin/${ARCH}
    DESTDIR_NAME=arm64-v8a
elif [ $ARCH = arm-linux-androideabi ]; then
    export TOOL_PATH=${ANDROID_TOOLCHAIN}/bin/${ARCH}
    DESTDIR_NAME=armeabi${V7A}
elif [ $ARCH = mipsel-linux-android ]; then
    export TOOL_PATH=${ANDROID_TOOLCHAIN}/bin/${ARCH}
    DESTDIR_NAME=mips
elif [ $ARCH = "mips64el-linux-android" ]; then
    export TOOL_PATH=${ANDROID_TOOLCHAIN}/bin/${ARCH}
    LIB_SUFFIX=64
    DESTDIR_NAME=mips64
    CONFIG_EXTRA="--disable-asm"
elif [ $ARCH = x86 ]; then
    export TOOL_PATH=${ANDROID_TOOLCHAIN}/bin/i686-linux-android
    DESTDIR_NAME=x86
elif [ $ARCH = x86_64 ]; then
    export TOOL_PATH=${ANDROID_TOOLCHAIN}/bin/x86_64-linux-android
    LIB_SUFFIX=64
    DESTDIR_NAME=x86_64
else
    echo "$0: invalid $ARCH" 1>&2
    exit 1
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

BUILDDIR="${ROOTDIR}/jni/${DESTDIR_NAME}"

# TODO: use `cross` to setup the environment and build MK

export pkg_configure_flags="--host=${ARCH} --disable-shared"
export pkg_make_flags=-j2
export pkg_prefix=${BUILDDIR}

(
    cd "${ROOTDIR}/../../"
    test -f Makefile && make clean
    echo "Configure with --host=${ARCH} and toolchain ${ANDROID_TOOLCHAIN}"
    test -x ${ROOTDIR}/../../configure || (cd ${ROOTDIR}/../.. && ./autogen.sh)
    ${ROOTDIR}/../../configure -q --host=${ARCH} --with-sysroot=${SYSROOT} \
      --with-libevent=${BUILDDIR} --with-geoip=${BUILDDIR} \
      --with-openssl=${BUILDDIR} --disable-examples \
      --prefix=${BUILDDIR} --disable-shared
    make V=0 -j2
    echo "Installing library in ${BASEDIR}/build/${ANDROID_TOOLCHAIN}"
    # The rationale of the following algorithm is to install-strip and do
    # only install headers we want and do not install extra stuff.
    # See: https://github.com/measurement-kit/measurement-kit/pull/274/files
    make install-strip
    rm -rf ${BUILDDIR}/include
    make install-data-am DESTDIR=${BUILDDIR}/
    rm -rf ${BUILDDIR}/usr
    rm -rf ${BUILDDIR}/share
    rm -rf ${BUILDDIR}/bin
    rm -rf ${BUILDDIR}/lib/*.la
    rm -rf ${BUILDDIR}/lib/pkgconfig
)
