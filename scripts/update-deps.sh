#!/bin/sh -e
# Clone specific tags from github and update local copy

get() {
  branch=$4
  [ -z "$branch" ] && branch=master
  git clone --depth 50 -b $branch https://github.com/$1 tmp/$1
  (cd tmp/$1 && git checkout $2)
  (cd tmp/$1 && git archive --prefix=$3/ HEAD) | \
    (cd src/ext/ && tar -xf-)
}

rm -rf tmp/*
get boostorg/assert boost-1.59.0 boost/assert
get boostorg/config boost-1.59.0 boost/config
get boostorg/core boost-1.59.0-8-g3add966 boost/core
get boostorg/detail boost-1.59.0 boost/detail
get boostorg/iterator boost-1.59.0 boost/iterator
get boostorg/mpl boost-1.59.0 boost/mpl
get boostorg/predef boost-1.59.0 boost/predef
get boostorg/preprocessor boost-1.59.0 boost/preprocessor
get boostorg/smart_ptr boost-1.59.0 boost/smart_ptr
get boostorg/static_assert boost-1.59.0 boost/static_assert
get boostorg/throw_exception boost-1.59.0 boost/throw_exception
get boostorg/type_traits boost-1.59.0 boost/type_traits
get boostorg/typeof boost-1.59.0 boost/typeof
get boostorg/utility boost-1.59.0 boost/utility
get jbeder/yaml-cpp release-0.5.2-16-g97d56c3 yaml-cpp
get joyent/http-parser v2.6.0 http-parser
get philsquared/Catch v1.2.1 Catch
rm -rf tmp
