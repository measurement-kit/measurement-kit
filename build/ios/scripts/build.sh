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
    # Note: the precompiled dependencies MAY be downloadable from a previous
    # release, what matters to decide if they're outdated is the `spec` dir that
    # you can find inside each tarball and contains version number info
    DEPS_URL=https://github.com/measurement-kit/measurement-kit/releases/download/v0.2.0-alpha/ios-dependencies-20160607T165015Z.tgz
    DEPS_FILE=$(basename $DEPS_URL)
    curl --progress-bar -LO $DEPS_URL
    curl --progress-bar -LO $DEPS_URL.asc
    gpg2 --verify $DEPS_FILE.asc
    tar -xzf $DEPS_FILE
    rm $DEPS_FILE $DEPS_FILE.asc
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
