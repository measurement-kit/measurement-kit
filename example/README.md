# Usage examples

The [ffi](ffi) folder contains usage examples of Measurement Kit C-like FFI
friendly API defined in [measurement_kit/ffi.h](
../include/measurement_kit/ffi.h). These are low level examples that interact
with MK by passing around serialized JSON documents.

The [nettests](nettests) folder shows how to use MK's legacy C++14 API in
distinct use cases. We currently do not have examples for the [new C++14 API](
../include/measurement_kit/cxx14.hpp) yet; however, you can perhaps look
into [how nettests is implemented in terms of the C++14 API](
../include/measurement_kit/nettests/legacy.hpp) to see how you can
write code using such new API.

Also reading [the source code of the measurement_kit binary](
../src/measurement_kit) may be of interests.
