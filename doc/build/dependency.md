# Building dependencies

## Overview

The objective of building dependencies is to obtain two signed
tarballs, one for iOS and one for Android, that can be downloaded
during the respective build processes, so to avoid compiling all
dependencies each time.

Dependencies are built using the `./build/dependency` script whose
wrappers for Android and iOS are, respectively,
`./build/android/dependency` and `./build/ios/dependency`.

The `dependency` script reads the proper package build specifications
from `./build/spec`, possibly downloads the sources (see below),
possibly applies patches, and builds the dependency. The wrapper scripts
set environment variables such that the build dependency can be integrated
into the iOS or Android build process.

Some dependencies are vendored at `src/third_party`. The `dependency`
script recognizes them and do not attempt to dowload their sources.

## Updating vendored dependencies

The workflow is the following:

1. update `./build/spec/foo` when there is a new release

2. generate the diff between the current status of a dependency
   and the previous pristine import using the
   `./build/vendor/get_local_patches foo` command.

3. run `./build/vendor/import foo` to download a pristine version
   of the new release under `./src/third_party`, and commit
   the results on the current branch.

3. apply the previously saved diffs using `git am` and fix conflicts. For
   each diff, commit when conflicts are fixed.

4. Make sure you can successfully build and cross build the new
   dependency. If necessary, apply more patches :-).

Note that currently you will need to have OpenBSD signify installed
on your system to verify the libressl tarball. You can easily get
it for macOS (`signify-osx` on Homebrew) and Debian (`signify-openbsd`
available from the standard repositories).

## Cross-compiling dependencies

To cross compile a dependency (e.g. `libressl`) for Android you can
use this command:

```
./build/android/dependency /path/to/ndk libressl
```

Where `/path/to/ndk` is the path where the NDK has been installed,
`$HOME/Library/Android/sdk/ndk-bundle` on macOS if you have installed
the NDK using Android studio. See [Android specific documentation](android.md)
for more information.

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

This may not necessarily be the best way to cross compile MK for the Android
platform, and we may drop suppot for that in the future.

## Submitting and fetching dependencies

You should upload the tarballs and their signature in the
[measurement-kit/dependencies](https://github.com/measurement-kit/dependencies)
repository. Use the latest release for that (depending on the maturity
stage such release may either be considered stable or unstable).

Then, you can download such dependencies using the `./build/fetch-dep` script
and, in particular, its iOS and Android wrappers.

For example, the following command

```
./build/android/fetch-dep -ps jni
```

will download all Android related files (i.e. files containing the string `jni`
into their name) from the latest release, verify their digital signature and
install them locally just like they have been compiled now.

Similarly, you should use this command for iOS

```
./build/ios/fetch-dep -ps iphone
```

that downloads only files containing `iphone` in their name.

Some dependencies, such as libevent, require that you either have compiled
other dependencies (such as libressl) or that you have fetched them just when
compiling as explained above.

## Creating the dependencies archives

To create the dependencies archives (i.e. the archives actually processed
by the MK build process), just download all the platform related already
compiled and published dependencies, e.g.

```
./build/android/fetch-dep -ps jni
```

Then create a single archive containing all the compiled artifacts:

```
./build/android/archive-deps
```

This will create an archive and it signature, and you should upload both to the
[measurement-kit/dependencies](https://github.com/measurement-kit/dependencies)
repository, in the latest release.
