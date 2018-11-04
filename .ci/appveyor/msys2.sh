#!/bin/sh
set -ex
BASE_URL=https://github.com/measurement-kit/prebuilt/releases/download
CHANNEL=testing
CURL_VERSION=7.61.1-9
CURL=mingw-curl-$CURL_VERSION
GEOIP_VERSION=1.6.12-9
GEOIP=mingw-geoip-api-c-$GEOIP_VERSION
LIBEVENT_VERSION=2.1.8-9
LIBEVENT=mingw-libevent-$LIBEVENT_VERSION
LIBMAXMINDDB_VERSION=1.3.2-9
LIBMAXMINDDB=mingw-libmaxminddb-$LIBMAXMINDDB_VERSION
LIBRESSL_VERSION=2.7.4-9
LIBRESSL=mingw-libressl-$LIBRESSL_VERSION
curl -fsSLO $BASE_URL/$CHANNEL/$CURL.tar.gz
curl -fsSLO $BASE_URL/$CHANNEL/$GEOIP.tar.gz
curl -fsSLO $BASE_URL/$CHANNEL/$LIBEVENT.tar.gz
curl -fsSLO $BASE_URL/$CHANNEL/$LIBMAXMINDDB.tar.gz
curl -fsSLO $BASE_URL/$CHANNEL/$LIBRESSL.tar.gz
tar -xzf $CURL.tar.gz
tar -xzf $GEOIP.tar.gz
tar -xzf $LIBEVENT.tar.gz
tar -xzf $LIBMAXMINDDB.tar.gz
tar -xzf $LIBRESSL.tar.gz
./autogen.sh
PREFIX=MK_DIST/mingw/
ARCH=x86_64
./configure --with-geoip=$PREFIX/geoip-api-c/$GEOIP_VERSION/$ARCH              \
            --with-libevent=$PREFIX/libevent/$LIBEVENT_VERSION/$ARCH           \
            --with-openssl=$PREFIX/libressl/$LIBRESSL_VERSION/$ARCH            \
            --with-libcurl=$PREFIX/curl/$CURL_VERSION/$ARCH                    \
            --with-libmaxminddb=$PREFIX/libmaxminddb/$LIBMAXMINDDB_VERSION/$ARCH \
            --with-ca-bundle=test/fixtures/saved_ca_bundle.pem                 \
            --disable-shared --disable-dependency-tracking
make -j`nproc` all
make -j`nproc` check TESTS=
make -j6 check || {
  if [ -f ./test-suite.log ]; then
    cat ./test-suite.log
  fi
  exit 1
}
