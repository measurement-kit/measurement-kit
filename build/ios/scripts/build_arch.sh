#!/bin/sh
set -e

if [ $# -ne 2 ]; then
    echo "$0 platform arch" 1>&2
    echo "Example: $0 iphonesimulator i386"
    exit 1
fi

PLATFORM=$1
ARCH=$2

MINIOSVERSION="7.1"
if [ "$PLATFORM" = "iphoneos" ]; then
    EXTRA_CONFIG="--host=arm-apple-darwin14 --target=arm-apple-darwin14"
    MINVERSION="-miphoneos-version-min=$MINIOSVERSION"
elif [ "$ARCH" = "i386" ]; then
    EXTRA_CONFIG="--host=i386-apple-darwin14 --target=i386-apple-darwin14"
    MINVERSION="-mios-simulator-version-min=$MINIOSVERSION"
elif [ "$ARCH" = "x86_64" ]; then
    EXTRA_CONFIG="--host=x86_64-apple-darwin14 --target=x86_64-apple-darwin14"
    MINVERSION="-mios-simulator-version-min=$MINIOSVERSION"
else
    echo "$0: unsupported configuration" 1>&2
    exit 1
fi

ROOTDIR=$(cd $(dirname $0) && pwd -P)
SOURCEDIR="$ROOTDIR/../../../"
BUILDDIR="$ROOTDIR/../"

# We assume that the user has the iphoneos sdk installed
AVAIL_SDKS=`xcodebuild -showsdks | grep "iphoneos"`
FIRST_SDK=`echo "$AVAIL_SDKS" | head -n1`
SDKVERSION=`echo "$FIRST_SDK" | cut -d\  -f2`
echo "Using this SDK version: $SDKVERSION"

DEVELOPER=$(xcode-select -print-path)

export PATH="${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/:${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/usr/bin/:${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin:${DEVELOPER}/usr/bin:${PATH}"

export CC="$(xcrun -find -sdk ${PLATFORM} cc) -arch ${ARCH} $MINVERSION"
export CXX="$(xcrun -find -sdk ${PLATFORM} g++) -arch ${ARCH} $MINVERSION"
export CPPFLAGS="-isysroot $(xcrun -sdk ${PLATFORM} --show-sdk-path)"
export CFLAGS="-isysroot $(xcrun -sdk ${PLATFORM} --show-sdk-path)"
export CXXFLAGS="-isysroot $(xcrun -sdk ${PLATFORM} --show-sdk-path)"
export LDFLAGS="-isysroot $(xcrun -sdk ${PLATFORM} --show-sdk-path)"

export pkg_configure_flags="$EXTRA_CONFIG"
export pkg_prefix=$BUILDDIR/build/${PLATFORM}/${ARCH}/
export pkg_make_flags=-j4

(
    cd $SOURCEDIR
    ./build/dependency libressl
    ./build/dependency libevent
    ./build/dependency jansson
    ./build/dependency geoip
    test -x ./configure || ./autogen.sh
    ./configure -q --disable-shared \
                --disable-examples \
                --with-libevent=$BUILDDIR/build/${PLATFORM}/${ARCH}/ \
                --with-jansson=$BUILDDIR/build/${PLATFORM}/${ARCH}/ \
                --with-geoip=$BUILDDIR/build/${PLATFORM}/${ARCH}/ \
                --with-openssl=$BUILDDIR/build/${PLATFORM}/${ARCH}/ \
                --prefix=/ \
                $EXTRA_CONFIG
    make -j4 V=0
    make install-strip V=0 DESTDIR=$BUILDDIR/build/${PLATFORM}/${ARCH}/
    rm -rf $BUILDDIR/build/${PLATFORM}/${ARCH}/include/
    make install-data-am V=0 DESTDIR=$BUILDDIR/build/${PLATFORM}/${ARCH}/
    make distclean
)
