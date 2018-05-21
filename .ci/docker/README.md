# Build scripts for Travis CI using Docker

The scripts in this directory are used to build Measurement
Kit in Travis-CI using Docker.

With Docker installed on your local machine, you can basically
reproduce the Travis CI builds by setting the same flags that
are set by [.travis.yml](../../.travis.yml) and the running
the [setup](setup) Unix script.

Specifically, these scripts are a fork of [measurement-kit/docker-ci](
https://github.com/measurement-kit/docker-ci) tailored for the
Measurement Kit case.

We look forward to switching over to [measurement-kit/docker-ci](
https://github.com/measurement-kit/docker-ci) once the build of MK
can be further simplified. That is, when one needs to invoke less
Unix scripts (like `./script/cmake/run`) to initialize the build. To
this end, we will basically need to move this functionality from
Unix scripts into the top-level `CMakeLists.txt` file.
