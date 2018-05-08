#!/bin/sh
set -ex
BASE_URL=https://github.com/measurement-kit/prebuilt/releases/download
CHANNEL=testing
GEOIP_VERSION=1.6.12-2
GEOIP=mingw-geoip-api-c-$GEOIP_VERSION
LIBEVENT_VERSION=2.1.8-3
LIBEVENT=mingw-libevent-$LIBEVENT_VERSION
LIBRESSL_VERSION=2.6.4-3
LIBRESSL=mingw-libressl-$LIBRESSL_VERSION
curl -LsO $BASE_URL/$CHANNEL/$GEOIP.tar.gz
curl -LsO $BASE_URL/$CHANNEL/$LIBEVENT.tar.gz
curl -LsO $BASE_URL/$CHANNEL/$LIBRESSL.tar.gz
tar -xzf $GEOIP.tar.gz
tar -xzf $LIBEVENT.tar.gz
tar -xzf $LIBRESSL.tar.gz
./autogen.sh
PREFIX=MK_DIST/mingw/
ARCH=x86_64
./configure --with-geoip=$PREFIX/geoip-api-c/$GEOIP_VERSION/$ARCH              \
            --with-libevent=$PREFIX/libevent/$LIBEVENT_VERSION/$ARCH           \
            --with-openssl=$PREFIX/libressl/$LIBRESSL_VERSION/$ARCH            \
            --with-ca-bundle=test/fixtures/saved_ca_bundle.pem                 \
            --disable-shared $PKG_CONFIGUREFLAGS
make V=0 -j$(nproc)
