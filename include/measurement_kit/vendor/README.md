# include/measurement_kit/vendor

Since MK v0.9.0-alpha.10 I have started writing small header-only libraries
inside their own repository. This provides the advantage that development
is much faster (since they are small, CI converges in a few seconds).

Such libraries are then vendored into a MK release and compiled inside MK.

They all have a similar, blocking C only API that is easy to wrap in the
target languages that we care about (i.e. ObjectiveC, Java, Go). Wrappers
will be implemented in the proper wrapping repository.

If you use the APIs in this folder, you accept the fact that they MAY
change at every MK release. The only API that provides stability guarantees
is currently the FFI API in [ffi.h](../ffi.h).
