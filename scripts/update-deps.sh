#!/bin/sh -e
# Clone specific tags from github and update local copy

get() {
  git clone https://github.com/$1 tmp/$1
  (cd tmp/$1 && git checkout $2)
  (cd tmp/$1 && git archive --prefix=$3/ HEAD) | \
    (cd src/ext/ && tar -xf-)
}

rm -rf tmp/*
get philsquared/Catch 76edbc1 Catch
get boostorg/assert boost-1.56.0 boost/assert
get boostorg/config boost-1.56.0 boost/config
get bassosimone/libight-boost-core boost-1.56.0-5-g348f24b boost/core
get boostorg/detail boost-1.56.0 boost/detail
get boostorg/iterator boost-1.56.0 boost/iterator
get boostorg/mpl boost-1.56.0 boost/mpl
get boostorg/predef boost-1.58.0~3^2~2 boost/predef
get boostorg/preprocessor boost-1.56.0 boost/preprocessor
get boostorg/smart_ptr boost-1.56.0-18-g8afd3be boost/smart_ptr
get boostorg/static_assert boost-1.56.0 boost/static_assert
get boostorg/throw_exception boost-1.56.0 boost/throw_exception
get boostorg/type_traits boost-1.56.0 boost/type_traits
get boostorg/typeof boost-1.56.0 boost/typeof
get boostorg/utility boost-1.56.0 boost/utility
get joyent/http-parser v2.1-47-g1b31580 http-parser
get measurement-kit/libevent release-2.0.22-stable-18-gf1feb10 libevent
get bassosimone/yaml-cpp master yaml-cpp
get akheron/jansson master jansson
get measurement-kit/libmaxminddb master libmaxminddb
rm -rf tmp
