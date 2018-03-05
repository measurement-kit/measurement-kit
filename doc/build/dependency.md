# Building dependencies

## Overview

The objective of building dependencies is to obtain two signed
tarballs, one for iOS and one for Android, that can be downloaded
during the respective build processes, so to avoid compiling all
dependencies each time.

Dependencies are built using the `./build/dependency` script whose
wrappers for Android and iOS are, respectively,
`./build/android/dependency` and `./build/ios/dependency`.

In turn `./build/dependency` is nowadays mostly a wrapper around
specific build scripts located at `./script/build/<dependency>`, as
you will notice the first time you run `./build/dependency`, since
it warns you about this. In going forward, the objective is to remove
`./build/dependency` and the related scripts, but this cannot be
done in a single step. Better to proceed piecemeal.

The wrapper scripts set specific environment variables such that the
dependency build can be integrated into the iOS or Android build process.

## Cross-compiling dependencies

To cross compile a dependency (e.g. `libressl`) for Android you can
use this command:

```
./build/android/dependency /path/to/ndk libressl
```

Where `/path/to/ndk` is the path where the NDK has been installed,
`$HOME/Library/Android/sdk/ndk-bundle` on macOS if you have installed
the NDK using Android studio. See [Android specific documentation](
../../script/build#cross-compile-for-android) for more information.

The result would be a tarball in the repository toplevel directory
for each Android architecture of interest.

You should create the digital signature of the tarball using:

```
./build/sign $tarball
```

Similarly, for iOS do:

```
./build/ios/dependency libressl
```

Assuming all the dependencies have been compiled, you can cross compile
MeasurementKit for iOS as it was a dependency using this command:

```
./build/ios/dependency mk
```

and for Android:

```
./build/android/dependency /path/to/ndk mk
```

(This may not necessarily be the best way to cross compile MK for the Android
platform, and we may drop suppot for that in the future.)

## Submitting and fetching dependencies

You should upload the tarballs and their signature in the
[measurement-kit/dependencies](https://github.com/measurement-kit/dependencies)
repository. Use the latest release for that (depending on the maturity
stage such release may either be considered stable or unstable).

Then, you can download such dependencies using the `./build/fetch-dep` script
and, in particular, its iOS wrapper.

For example, the following command

```
./build/ios/fetch-dep -ps iphone
```

will download all Android related files (i.e. files containing the string
`iphone` into their name) from the latest release, verify their digital
signature and install them locally just like they have been compiled now.

Some dependencies, such as libevent, require that you either have compiled
other dependencies (such as libressl) or that you have fetched them just when
compiling as explained above.

## Creating the dependencies archives

To create the dependencies archives (i.e. the archives actually processed
by the MK build process), just download all the platform related already
compiled and published dependencies, e.g.

```
./build/ios/fetch-dep -ps iphone
```

Then create a single archive containing all the compiled artifacts:

```
./build/ios/archive-deps
```

This will create an archive and it signature, and you should upload both to the
[measurement-kit/dependencies](https://github.com/measurement-kit/dependencies)
repository, in the latest release.

(TODO(bassosimone): document more thoroughly the simplified process of
releasing for Android and soon for iOS.)
