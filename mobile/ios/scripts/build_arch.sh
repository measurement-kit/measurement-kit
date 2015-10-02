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

DEVELOPER=$(xcode-select -print-path)
REPOROOT=$(pwd)
MINIOSVERSION="7.1"
SDKVERSION="9.0"

export PATH="${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/:${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/usr/bin/:${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin:${DEVELOPER}/usr/bin:${PATH}"

export CC="/usr/bin/gcc -arch ${ARCH} -miphoneos-version-min=${MINIOSVERSION}"
export CXX="/usr/bin/g++ -arch ${ARCH} -miphoneos-version-min=${MINIOSVERSION}"
export CPPFLAGS="-isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDKVERSION}.sdk"
export CFLAGS="-isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDKVERSION}.sdk"
export CXXFLAGS="-isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDKVERSION}.sdk"

(
    cd measurement-kit
    test -x ./configure || autoreconf -i
    ./configure --disable-shared \
                --with-libevent=builtin \
                --with-libyaml-cpp=builtin \
                --with-libboost=builtin \
                --prefix=/ \
                $EXTRA_CONFIG
    make -j4 V=0
    make install V=0 DESTDIR=$REPOROOT/build/${PLATFORM}/${ARCH}
    make distclean
)

# Build libboost.a stub necessary to create boost.framework
install -d $REPOROOT/build/${PLATFORM}/${ARCH}/lib
$CXX $CPPFLAGS $CXXFLAGS -c boost/boost.cpp \
  -o $REPOROOT/build/${PLATFORM}/${ARCH}/lib/boost.o
ar rv $REPOROOT/build/${PLATFORM}/${ARCH}/lib/libboost.a \
  $REPOROOT/build/${PLATFORM}/${ARCH}/lib/boost.o
rm $REPOROOT/build/${PLATFORM}/${ARCH}/lib/boost.o
ranlib $REPOROOT/build/${PLATFORM}/${ARCH}/lib/libboost.a
