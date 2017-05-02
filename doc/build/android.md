# Cross-compiling MK dependencies for Android

We have [a specific repository](https://github.com/measurement-kit/android-libs)
for compiling MeasurementKit for Android. This file explains how you can get
the Android NDK and how to use it to cross compile dependencies for Android.

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

### On macOS using brew

You can install it with brew. Just type the following:

    $ brew install android-ndk

but from our experience we recommend to [install it using Android studio](
https://developer.android.com/ndk/guides/index.html#download-ndk)
since this is the most reliable way to get a working ndk-build.
On macOS, Android studio installs the ndk-build at 
`~/Library/Android/sdk/ndk-bundle/ndk-build`.

## Cross compiling MeasurementKit dependencies for Android

The `./dependency` script allows to create the required custom
toolchains and to cross-compile the dependencies for all the architectures
available for Android. If you run the script without arguments, it will
print the options it accepts and the available Android architectures for
which you can cross compile:

    $ ./build/android/dependency
    usage: ./build/android/dependency NDK_DIR spec

To cross-compile you need to tell the script where did you install the NDK
and which dependency `spec` you want to build. For example, on macOS you can use
the following command line:

    $ ./build/android/dependency ~/Library/Android/sdk/ndk-bundle libevent

to build libevent.

In the above examples we have shown the path to the Android NDK on macOS. If
you followed the instructions for Linux, you should have written instead:

    $ ./build/android/dependency $HOME/Android/android-ndk-r10e/ [spec]

See also [the documentation of the core script used to build
dependencies](dependency.md).
