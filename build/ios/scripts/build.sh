#!/bin/sh
set -e
ROOTDIR=$(cd `dirname "$0"` && pwd -P)

if [ $# -ge 1 ]; then
    ARCHS=$@
else
    ARCHS="i386 x86_64 armv7 armv7s arm64"
fi

echo "Downloading and verifying precompiled dependencies from github"
(
    set -e # just in case
    cd $ROOTDIR/..
    curl --progress-bar -LO https://github.com/measurement-kit/measurement-kit/releases/download/v0.2.0-alpha/ios-dependencies.tgz
    curl --progress-bar -LO https://github.com/measurement-kit/measurement-kit/releases/download/v0.2.0-alpha/ios-dependencies.tgz.asc
    gpg2 --verify ios-dependencies.tgz.asc
    tar -xzf ios-dependencies.tgz
    rm ios-dependencies.tgz ios-dependencies.tgz.asc
)

echo "Building for this architectures: $ARCHS"

for ARCH in ${ARCHS}; do
    if [ "${ARCH}" == "i386" ] || [ "${ARCH}" == "x86_64" ]; then
        PLATFORM="iphonesimulator"
    else
        PLATFORM="iphoneos"
    fi
    $ROOTDIR/build_arch.sh ${PLATFORM} ${ARCH}
done

$ROOTDIR/build_frameworks.sh
