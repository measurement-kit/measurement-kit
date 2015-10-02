# Building MeasurementKit for iOS

MeasurementKit is an experimental library that provides common functionalities
useful to implement open measurement tools on mobile platforms.

This repository contains the code to cross-compile MeasurementKit for iOS.

To build for iOS you need to have a MacOSX 10.9.5 or higher system
with Xcode installed (older systems may be able to compile it but
this has not been tested by us).

## How to clone the repository

To clone properly MeasurementKit repository, make sure that you specify the
`--recursive` command line flag, as in:

    git clone --recursive https://github.com/measurement-kit/measurement-kit-build-ios

Such flag tells git to clone not only the MeasurementKit repository, but also
the [submodules](http://git-scm.com/docs/git-submodule) contained therein.

Alternatively, once you have cloned the repository, you can checkout all
the submodules using:

    git submodule update --init --recursive

## How to build MeasurementKit for iOS

The repository contains empty frameworks for MeasurementKit and for
all its dependencies. You just need to run

    ./scripts/build.sh

to compile everything and fill all the empty framework directories
with static libraries for iPhoneOS and iPhoneSimulator as well as with
all the required headers. You can then copy these frameworks inside
you project and import them in Xcode to integrate with MeasurementKit.

## How to update to the latest version of MeasurementKit

Create a work branch. Then execute these steps.

```
cd measurement-kit
git submodule deinit -f .
git checkout master
git pull
git submodule update --init --recursive
cd ..
git commit -am "Update measurement-kit subrepo"
```

Then push the branch and open a pull request.

## How to update to a specific branch for testing purposes

Assume `feature/foobar` is a branch you want to test. Then do these steps.

```
git fetch
git submodule deinit -f .
git checkout feature/foobar
git submodule update --init --recursive 
```

Then test the branch. Once you're done return to master like this.

```
git submodule deinit -f .
git checkout master
git submodule update --init --recursive
```
