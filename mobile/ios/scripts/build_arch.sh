#!/bin/sh -e

if [ $# -ne 2 ]; then
    echo "$0 platform arch" 1>&2
    echo "Example: $0 iPhoneSimulator i386"
    exit 1
fi

PLATFORM=$1
ARCH=$2

if [ "$PLATFORM" = "iPhoneOS" ]; then
    EXTRA_CONFIG="--host=arm-apple-darwin14 --target=arm-apple-darwin14"
fi

ROOTDIR=$(cd $(dirname $0) && pwd -P)
SOURCEDIR="$ROOTDIR/../../../"
BUILDDIR="$ROOTDIR/../"

# XXX We assume that the user has the iphoneos sdk installed
AVAIL_SDKS=`xcodebuild -showsdks | grep "iphoneos"`
FIRST_SDK=`echo "$AVAIL_SDKS" | head -n1`
SDKVERSION=`echo "$FIRST_SDK" | cut -d\  -f2`
echo "Using this SDK version: $SDKVERSION"

DEVELOPER=$(xcode-select -print-path)
MINIOSVERSION="7.1"

export PATH="${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/:${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/usr/bin/:${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin:${DEVELOPER}/usr/bin:${PATH}"

export CC="/usr/bin/gcc -arch ${ARCH} -miphoneos-version-min=${MINIOSVERSION}"
export CXX="/usr/bin/g++ -arch ${ARCH} -miphoneos-version-min=${MINIOSVERSION}"
export CPPFLAGS="-isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDKVERSION}.sdk"
export CFLAGS="-isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDKVERSION}.sdk"
export CXXFLAGS="-isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDKVERSION}.sdk"

(
    cd $SOURCEDIR
    test -x ./configure || ./autogen.sh
    ./configure -q --disable-shared \
                --disable-examples \
                --with-libevent=builtin \
                --with-yaml-cpp=builtin \
                --with-boost=builtin \
                --with-jansson=builtin \
                --with-libmaxminddb=builtin \
                --prefix=/ \
                $EXTRA_CONFIG
    make -j4 V=0
    make install-strip V=0 DESTDIR=$BUILDDIR/build/${PLATFORM}/${ARCH}
    rm -rf $BUILDDIR/build/${PLATFORM}/${ARCH}/include
    make install-data-am V=0 DESTDIR=$BUILDDIR/build/${PLATFORM}/${ARCH}
    make distclean
)
