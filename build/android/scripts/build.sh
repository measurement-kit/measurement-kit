#!/bin/sh -e

ROOTDIR=$(cd $(dirname $(dirname $0)) && pwd -P)
if [ $? -ne 0 ]; then
    echo "$0: cannot determine root directory" 1>&2
    exit 1
fi

ALL_ARCHS="aarch64-linux-android arm-linux-androideabi arm-linux-androideabi-v7a mipsel-linux-android mips64el-linux-android x86 x86_64"

if [ $# -eq 3 ]; then
    NDK_DIR=$1
    ALL_ARCHS=$2
    API=$3
elif [ $# -eq 1 -o $# -eq 2 ]; then
    NDK_DIR=$1
    if [ $# -eq 2 ]; then
        API=$2
    else
        API=21
    fi
else
    echo "Usage: $0 NDK_DIR [[ARCH] API]" 1>&2
    echo "  NDK_DIR: path where NDK is installed" 1>&2
    echo "  ARCH: aarch64-linux-android arm-linux-androideabi mipsel-linux-android mips64el-linux-android x86 x86_64" 1>&2
    echo "  API: 1 ... 21" 1>&2
    echo "If you omit API, 21 is used." 1>&2
    echo "If you omit ARCH, all archs are compiled." 1>&2
    echo "Example usage (on MacOS using brew):" 1>&2
    echo "  $0 /usr/local/Cellar/android-ndk/r10e/ x86 16" 1>&2
    echo "or" 1>&2
    echo "  $0 /usr/local/Cellar/android-ndk/r10e/ 16" 1>&2
    echo "or" 1>&2
    echo "  $0 /usr/local/Cellar/android-ndk/r10e/" 1>&2
    exit 1
fi

echo "Downloading and verifying precompiled dependencies from github"
(
    set -e # just in case
    cd $ROOTDIR
    # Note: the precompiled dependencies MAY be downloadable from a previous
    # release, what matters to decide if they're outdated is the `spec` dir that
    # you can find inside each tarball and contains version number info
    DEPS_URL=https://github.com/measurement-kit/measurement-kit/releases/download/v0.2.0-alpha/android-dependencies-20160607T203701Z.tgz
    DEPS_FILE=$(basename $DEPS_URL)
    curl --progress-bar -LO $DEPS_URL
    curl --progress-bar -LO $DEPS_URL.asc
    gpg2 --verify $DEPS_FILE.asc
    tar -xzf $DEPS_FILE
    rm $DEPS_FILE $DEPS_FILE.asc
)

for ARCH in $ALL_ARCHS; do
  (
    cd $ROOTDIR
    if [ ! -d toolchain/${ARCH}-${API} ]; then
        ${ROOTDIR}/scripts/make_toolchain.sh ${NDK_DIR} ${ARCH} ${API}
    fi
    ${ROOTDIR}/scripts/build_target.sh ${ARCH} ${API}
  )
done
