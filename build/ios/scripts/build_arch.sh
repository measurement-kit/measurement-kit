#!/bin/sh
set -e

# TODO: rewrite this script in terms of `../cross` and `../dependency`

if [ $# -ne 2 ]; then
    echo "$0 platform arch" 1>&2
    echo "Example: $0 iphonesimulator i386"
    exit 1
fi

PLATFORM=$1
ARCH=$2

MINIOSVERSION="7.1"
if [ "$PLATFORM" = "iphoneos" ]; then
    EXTRA_CONFIG="--host=arm-apple-darwin14 --target=arm-apple-darwin14 --disable-shared"
    MINVERSION="-miphoneos-version-min=$MINIOSVERSION"
elif [ "$ARCH" = "i386" ]; then
    EXTRA_CONFIG="--host=i386-apple-darwin14 --target=i386-apple-darwin14 --disable-shared"
    MINVERSION="-mios-simulator-version-min=$MINIOSVERSION"
elif [ "$ARCH" = "x86_64" ]; then
    EXTRA_CONFIG="--host=x86_64-apple-darwin14 --target=x86_64-apple-darwin14 --disable-shared"
    MINVERSION="-mios-simulator-version-min=$MINIOSVERSION"
else
    echo "$0: unsupported configuration" 1>&2
    exit 1
fi

ROOTDIR=$(cd $(dirname $0) && pwd -P)
SOURCEDIR="$ROOTDIR/../../../"
DESTDIR="$ROOTDIR/../tmp/${PLATFORM}/${ARCH}/"

export CC="$(xcrun -find -sdk ${PLATFORM} cc)"
export CXX="$(xcrun -find -sdk ${PLATFORM} g++)"
export CPPFLAGS="-arch ${ARCH} -isysroot $(xcrun -sdk ${PLATFORM} --show-sdk-path)"
export CFLAGS="-arch ${ARCH} $MINVERSION -isysroot $(xcrun -sdk ${PLATFORM} --show-sdk-path)"
export CXXFLAGS="-arch ${ARCH} $MINVERSION -isysroot $(xcrun -sdk ${PLATFORM} --show-sdk-path)"
export LDFLAGS="-arch ${ARCH} $MINVERSION -isysroot $(xcrun -sdk ${PLATFORM} --show-sdk-path)"

export pkg_configure_flags="$EXTRA_CONFIG"
export pkg_prefix="$DESTDIR"
export pkg_make_flags=-j4

(
    cd $SOURCEDIR
    test -x ./configure || ./autogen.sh
    ./configure -q --disable-examples \
                --with-libevent=$DESTDIR/ \
                --with-geoip=$DESTDIR/ \
                --with-openssl=$DESTDIR/ \
                --prefix=/ \
                $EXTRA_CONFIG
    make -j4 V=0
    make install-strip V=0 DESTDIR=$DESTDIR/
    rm -rf $DESTDIR/include/
    make install-data-am V=0 DESTDIR=$DESTDIR/
    make distclean
)
