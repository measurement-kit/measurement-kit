# Build measurement-kit for Android

This repository contains the scripts needed to cross compile
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

## Creating a custom toolchain

Next, you need to [create a custom toolchain](
http://www.kandroid.org/ndk/docs/STANDALONE-TOOLCHAIN.html)
for the platform and API level that you want to target
using the `./scripts/make_toolchain.sh` command.

### On Linux

The following command creates a standalone
toolchain in `./toolchain/` for ARM-based Android devices
using API level 9 (i.e., Android 2.3).

    $ ./scripts/make_toolchain.sh $HOME/Android/android-ndk-r10e/ arm-linux-androideabi 9

The first argument is the path where NDK r10e is installed, the second
argument is the cross compiler type, the third argument is the API level.

### On MacOS using brew

The following command creates a standalone
toolchain in `./toolchain/` for ARM-based Android devices
using API level 9 (i.e., Android 2.3).

    $ ./scripts/make_toolchain.sh /usr/local/Cellar/android-ndk/r10e/ arm-linux-androideabi 9

The first argument is the path where NDK r10e is installed, the second
argument is the cross compiler type, the third argument is the API level.

## Cross-compiling

Once you created a custom toolchain, you can cross compile
measurement-kit using that toolchain using the `./scripts/build_target.sh`
script, which accepts as command line parameters the cross compiler
type and the API level. For example,

    $ ./scripts/build_target.sh arm-linux-androideabi 9

this script cross compiles measurement-kit under `./build` and puts
the compiled libraries and haders under `./dist`.

## Putting all together

You can execute both steps (creating a cross compiler and cross compiling) in
a single command using the `./build.sh` script. For example,

    $ ./build.sh /usr/local/Cellar/android-ndk/r10e/ arm-linux-androideabi 9

## TODO next

What is missing to continue this work is to compile and link
with measurement-kit and all its dependencies JNI code that can be
accessed from Java to use measurement-kit.
