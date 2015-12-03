#!/bin/sh -e
if [ $# -ne 2 ]; then
    echo "usage: $0 flavour srcdir" 1>&2
    exit 1
fi
flavour=$1
srcdir=$2
version=`git describe --tags`
tar -cjf measurement_kit-$flavour-$version.tar.bz2 $srcdir/
