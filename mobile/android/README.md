# Build measurement-kit for Android

This directory contains the scripts needed to cross compile
measurement-kit for Android platforms.

## Installing the NDK

### On Linux

The first step to compile measurement-kit for Android is to download the
NDK (Native Development Kit) from [developer.android.com](
https://developer.android.com/tools/sdk/ndk/index.html).  We have tested
this repository with version 10e of the NDK.

Then copy the downloaded file to `$HOME/Android` and from inside this directory
run the downloaded file to unpack the NDK:

    $ pwd
    /home/simone/Android
    $ ./android-ndk-r10e-linux-x86_64.bin

### On MacOS using brew

Just type the following:

    $ brew install android-ndk

## Cross compiling MeasurementKit for Android

The `./scripts/build.sh` script allows to create the required custom
toolchains and to cross-compile MeasurementKit for all the architectures
available for Android. If you run the script without arguments, it will
print the options it accepts and the available Android architectures for
which you can cross compile:

    $ ./scripts/build.sh
    [prints usage]

To cross-compile for all available architectures you need to tell the
script where did you install the NDK. For example, on MacOS you can use
the following command line:

    $ ./scripts/build.sh /usr/local/Cellar/android-ndk/r10e/

The above command compiles for Android with API 21. You can compile
for another API by providing the API number as the second argument on
the command line:

    $ ./scripts/build.sh /usr/local/Cellar/android-ndk/r10e/ 20

If you want to compile only for a specific architecture, add it before
the API number on the command line:

    $ ./scripts/build.sh /usr/local/Cellar/android-ndk/r10e/ \
                         arm-linux-androideabi 20

In the above examples we have shown the path to the Android NDK on MacOS. If
you followed the instructions for Linux, you should have written instead:

    $ ./scripts/build.sh $HOME/Android/android-ndk-r10e/ [options...]

This script is implemented by two helper scripts respectively called
`./scripts/make_toolchain.sh` and `./scripts/build_arch.sh`. The former
[creates a standalone cross-toolchain](
http://www.kandroid.org/ndk/docs/STANDALONE-TOOLCHAIN.html)
for a specific Android arch. The
latter cross-compiles MeasurementKit for the selected arch.

## Create the final archive

Run this command to create the final archive:

    $ ./scripts/make_archive.sh

Then use GPG to sign the release and publish it on GitHub:

    $ gpg --sign --armor -b $tarball
