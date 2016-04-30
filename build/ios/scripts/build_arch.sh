#!/bin/sh
set -e

if [ $# -ne 2 ]; then
    echo "$0 platform arch" 1>&2
    echo "Example: $0 iPhoneSimulator i386"
    exit 1
fi

PLATFORM=$1
ARCH=$2

OPT_FLAGS="-Os -g3"
MAKE_JOBS=2

ROOTDIR=$(cd $(dirname "$0") && pwd -P)
SOURCEDIR=$(cd "${ROOTDIR}/../../../" && pwd -P)
BUILDDIR=$(cd "${ROOTDIR}/../" && pwd -P)

mkdir -p "$BUILDDIR/build/${PLATFORM}/${ARCH}/"
DESTDIR=$(cd "$BUILDDIR/build/${PLATFORM}/${ARCH}/" && pwd -P)


MIOS_VERSION="7.1"
if [ $PLATFORM == "iphoneos" ]; then
    MIOS_VERSION="-miphoneos-version-min=$MIOS_VERSION"
else 
    MIOS_VERSION="-mios-simulator-version-min=$MIOS_VERSION"
fi

HOST_FLAGS="-arch ${ARCH} ${MIOS_VERSION} -isysroot $(xcrun -sdk ${PLATFORM} --show-sdk-path)"

if [ $PLATFORM == "iphoneos" ]; then
    CHOST="arm-apple-darwin"
else 
    if [ $ARCH == "i386" ]; then
        CHOST="i386-apple-darwin"
    else
        CHOST="x86_64-apple-darwin"
    fi
fi
export CC="$(xcrun -find -sdk ${PLATFORM} cc)"
export CXX="$(xcrun -find -sdk ${PLATFORM} g++)"
export CPP="$(xcrun -find -sdk ${PLATFORM} cpp)"

export CFLAGS="${HOST_FLAGS} ${OPT_FLAGS}"
export CXXFLAGS="${HOST_FLAGS} ${OPT_FLAGS}"
export LDFLAGS="${HOST_FLAGS}"

CONF_FLAGS="--host=${CHOST} --enable-static --disable-shared"
export pkg_configure_flags=${CONF_FLAGS}
export pkg_make_flags=-j${MAKE_JOBS}
export pkg_prefix=${DESTDIR}

(
    cd $SOURCEDIR
    ./build/dependency libressl
    ./build/dependency libevent
    ./build/dependency jansson
    ./build/dependency geoip
    test -x ./configure || ./autogen.sh
    ./configure -q --disable-shared \
                --disable-examples \
                --with-libevent=${DESTDIR} \
                --with-jansson=${DESTDIR} \
                --with-geoip=${DESTDIR} \
                --with-openssl=${DESTDIR} \
                --prefix=/ \
                ${CONF_FLAGS}
    make -j4 V=0
    make install-strip V=0 DESTDIR=${DESTDIR}/
    rm -rf ${DESTDIR}/include/
    make install-data-am V=0 DESTDIR=${DESTDIR}/
    make distclean
)
