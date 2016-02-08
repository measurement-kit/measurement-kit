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
get jbeder/yaml-cpp release-0.5.2-16-g97d56c3 yaml-cpp
get joyent/http-parser v2.6.0 http-parser
get philsquared/Catch v1.2.1 Catch
rm -rf tmp
