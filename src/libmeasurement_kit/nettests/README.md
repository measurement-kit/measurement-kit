# High level implementation of network tests

This folder is historically called `nettests` because before MK v0.9.0 it
was tightly coupled with the `nettests.hpp` API used by MK.

After MK v0.9.0, `nettests.hpp` is implemented in terms of the [FFI API](
../../../include/measurement_kit/ffi.h) and code in this folder generates
events emitted by the FFI API. However, the two, now-distinct parts of
MK still have the same naming for historical reasons.
