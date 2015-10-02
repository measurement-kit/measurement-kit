#!/bin/sh -e

./scripts/build_arch.sh iPhoneSimulator i386
./scripts/build_arch.sh iPhoneSimulator x86_64
./scripts/build_arch.sh iPhoneOS armv7
./scripts/build_arch.sh iPhoneOS armv7s
./scripts/build_arch.sh iPhoneOS arm64

./scripts/build_frameworks.sh
