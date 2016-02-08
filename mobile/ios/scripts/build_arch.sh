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
export LDFLAGS="-isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDKVERSION}.sdk"

(
    cd $SOURCEDIR
    export pkg_prefix=$BUILDDIR/build/${PLATFORM}/${ARCH}
    export pkg_configure_flags=$EXTRA_CONFIG
    ./build/dependency all
    test -x ./configure || autoreconf -i
    ./configure -q --disable-shared \
                --disable-examples \
                --with-libevent=${pkg_prefix} \
                --with-yaml-cpp=${pkg_prefix} \
                --with-boost=${pkg_prefix} \
                --with-jansson=${pkg_prefix} \
                --with-libmaxminddb=${pkg_prefix} \
                --with-Catch=${pkg_prefix} \
                --with-http-parser=${pkg_prefix} \
                --prefix=$BUILDDIR/build/${PLATFORM}/${ARCH} \
                $EXTRA_CONFIG
    make -j4 V=0
    make install
)
