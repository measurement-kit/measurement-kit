#!/bin/sh
set -e
ROOTDIR=$(cd `dirname "$0"` && pwd -P)

if [ $# -ge 1 ]; then
    ARCHS=$@
else
    ARCHS="i386 x86_64 armv7 armv7s arm64"
fi

echo "Building for this architectures: $ARCHS"

for ARCH in ${ARCHS}; do
    if [ "${ARCH}" == "i386" ] || [ "${ARCH}" == "x86_64" ]; then
        PLATFORM="iPhoneSimulator"
    else
        PLATFORM="iPhoneOS"
    fi
    $ROOTDIR/build_arch.sh ${PLATFORM} ${ARCH}
done

$ROOTDIR/build_frameworks.sh
