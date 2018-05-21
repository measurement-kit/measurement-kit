# Legacy docker-based build infrastructure

This directory contains legacy scripts used to run Docker in
Travis CI and possibly other CI environments.

A simplified version of these scripts tailored to CMake now
lives in [.ci/docker](../../.ci/docker). We will keep this
directory around as long as we have an autotools based build
that runs in CI environments. The scripts in here, in fact,
are tailored on running MK builds using autotools.
