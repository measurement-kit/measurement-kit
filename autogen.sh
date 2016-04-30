#!/bin/sh

set -e

slug() {
    echo $(echo $1|tr '/-' '_'|sed 's/^include_measurement_kit/mk/g')
}

gen_headers() {
    rm -f include/measurement_kit/*.hpp
    for name in $(ls include/measurement_kit/); do
        hh=include/measurement_kit/$name.hpp
        echo "// File autogenerated by \`$0\`, do not edit"                > $hh
        echo "#ifndef MEASUREMENT_KIT_$(echo $name|tr 'a-z' 'A-Z')_HPP"   >> $hh
        echo "#define MEASUREMENT_KIT_$(echo $name|tr 'a-z' 'A-Z')_HPP"   >> $hh
        for nn in $(ls include/measurement_kit/$name/); do
            echo "#include <measurement_kit/$name/$nn>"                   >> $hh
        done
        echo "#endif"                                                     >> $hh
    done

    echo "$(slug $1)_includedir = $1"
    echo "$(slug $1)_include_HEADERS = # Empty"
    for name in `ls $1`; do
        if [ ! -d $1/$name ]; then
            echo "$(slug $1)_include_HEADERS += $1/$name"
        fi
    done
    echo ""
    for name in `ls $1`; do
        if [ -d $1/$name ]; then
            gen_headers $1/$name
        fi
    done
}

gen_sources() {
    for name in `ls $2`; do
        if [ ! -d $2/$name ]; then
            if echo $name | grep -q '\.c[p]*$'; then
                echo "$1 += $2/$name"
            fi
        fi
    done
    for name in `ls $2`; do
        if [ -d $2/$name ]; then
            if [ "$2/$name" = "src/ext" ]; then
                continue  # do not descend into external sources
            fi
            gen_sources $1 $2/$name
        fi
    done
}

gen_executables() {
    for name in `ls $2`; do
        if [ ! -d $2/$name ]; then
            if echo $name | grep -q '\.c[p]*$'; then
                bin_name=$(echo $name | sed 's/\.c[p]*$//g')
                echo ""
                echo "if $3"
                echo "    $1 += $2/$bin_name"
                echo "endif"
                echo "$2/$bin_name" >> .gitignore
                echo "$(slug $2/$bin_name)_SOURCES = $2/$name"
                echo "$(slug $2/$bin_name)_LDADD = libmeasurement_kit.la"
            fi
        fi
    done
    for name in `ls $2`; do
        if [ -d $2/$name ]; then
            gen_executables $1 $2/$name $3
        fi
    done
}

get() {
  echo ""
  echo "> $3 (from github.com/$1)"
  branch=$4
  [ -z "$branch" ] && branch=master
  if [ ! -d src/ext/$3 ]; then
      git clone --depth 50 -b $branch https://github.com/$1 src/ext/$3
  else
      (cd src/ext/$3 && git checkout $branch && git pull)
  fi
  (cd src/ext/$3 && git checkout $2)
  echo ""
}

get_geoipdb() {
  echo ""
  base=https://download.maxmind.com/download/geoip/database
  if [ ! -f "test/fixtures/GeoIP.dat" ]; then
    wget -q $base/GeoLiteCountry/GeoIP.dat.gz -O test/fixtures/GeoIP.dat.gz
    gzip -d test/fixtures/GeoIP.dat.gz
  fi
  if [ ! -f "test/fixtures/GeoIPASNum.dat" ]; then
    wget -q $base/asnum/GeoIPASNum.dat.gz -O test/fixtures/GeoIPASNum.dat.gz
    gzip -d test/fixtures/GeoIPASNum.dat.gz
  fi
}

grep -v -E "^(test|example){1}/.*" .gitignore > .gitignore.new
echo test/fixtures/GeoIP.dat >> .gitignore.new
echo test/fixtures/GeoIPASNum.dat >> .gitignore.new
mv .gitignore.new .gitignore

echo "* Generating include.am"
echo "# Autogenerated by $0 on date $(date)"            > include.am
echo ""                                                >> include.am
gen_sources libmeasurement_kit_la_SOURCES src          >> include.am
echo ""                                                >> include.am
gen_headers include/measurement_kit                    >> include.am
gen_executables noinst_PROGRAMS example BUILD_EXAMPLES >> include.am
gen_executables ALL_TESTS test BUILD_TESTS             >> include.am

echo "* Updating .gitignore"
LC_ALL=C sort -u .gitignore > .gitignore.new
mv .gitignore.new .gitignore

echo "* Fetching dependencies that are build in any case"
get joyent/http-parser v2.6.0 http-parser
get philsquared/Catch v1.2.1 Catch
get nlohmann/json v1.1.0 json v1.1.0

echo "* Fetching geoip database"
get_geoipdb

echo "* Running 'autoreconf -i'"
autoreconf -i

echo "=== autogen.sh complete ==="
echo ""
echo "MeasurementKit is now ready to be compiled. To proceed you shall run"
echo "now the './configure' script that adapts the build to your system."
echo ""
echo "The './configure' script shall also check for external dependencies. "
echo "MeasurementKit external dependencies are:"
echo ""
for depname in `ls build/spec/|grep -v all`; do
    echo "    - $depname"
done
echo ""
echo "If any of these dependencies is missing, the './configure' script shall"
echo "stop and tell you how you could install it. Note that you can compile"
echo "any of these dependencies using the './build/dependency' script'. So, to"
echo "install e.g. libevent, type:"
echo ""
echo "./build/dependency libevent"
echo ""
