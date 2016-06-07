#!/bin/sh
set -e

ROOTDIR=$(cd $(dirname $0) && pwd -P)
EXTLIBRARIES="libevent libevent_pthreads libevent_openssl libcrypto libssl libGeoIP"

(
    cd "$ROOTDIR/.."

    # Clean old frameworks (if any)
    rm -rf Frameworks/*

    # Create framework folders
    for lib in $EXTLIBRARIES measurement_kit; do
        mkdir Frameworks/$lib.framework
    done

    # Copying x86 headers. This has no implications because measurement-kit
    # uses no machine-dependent headers.
    cp -Rp tmp/iPhoneSimulator/i386/include/measurement_kit \
        Frameworks/measurement_kit.framework/Headers

    # Lipo external libraries
    for lib in $EXTLIBRARIES; do 
        lipo -create -output Frameworks/$lib.framework/$lib \
          tmp/iPhoneOS/arm64/lib/$lib.a \
          tmp/iPhoneOS/armv7s/lib/$lib.a \
          tmp/iPhoneOS/armv7/lib/$lib.a \
          tmp/iPhoneSimulator/i386/lib/$lib.a \
          tmp/iPhoneSimulator/x86_64/lib/$lib.a
    done
    
    # Lipo measurement-kit library 
    lipo -create -output Frameworks/measurement_kit.framework/measurement_kit \
      tmp/iPhoneOS/arm64/lib/libmeasurement_kit.a \
      tmp/iPhoneOS/armv7s/lib/libmeasurement_kit.a \
      tmp/iPhoneOS/armv7/lib/libmeasurement_kit.a \
      tmp/iPhoneSimulator/i386/lib/libmeasurement_kit.a \
      tmp/iPhoneSimulator/x86_64/lib/libmeasurement_kit.a

    # Create fake header to make CocoaPod happy
    for lib in $EXTLIBRARIES; do
        install -d Frameworks/$lib.framework/Headers
        echo "/* Make CocoaPod happy */" > \
            Frameworks/$lib.framework/Headers/dummy_header.h
    done
)
