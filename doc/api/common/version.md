# NAME

`measurement_kit/common/version.h`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_VERSION_HPP
#define MEASUREMENT_KIT_COMMON_VERSION_HPP

#define MK_VERSION "0.8.0"
#define MK_VERSION_FULL "v0.8.0-6-abcdef"

#ifdef __cplusplus
extern "C" {
#endif

const char *mk_version(void);

const char *mk_version_full(void);

const char *mk_openssl_version(void);

const char *mk_libevent_version(void);

#ifdef __cplusplus
} // namespace mk
#endif
#endif
```

# DESCRIPTION

`MK_VERSION` is MK version expressed using semantic versioning.

`mk_version` returns MK version.

`mk_version_full` returns the result of `git describe --tags` as run when generating a release tarball or a development build. This is a more comprehensive (hence "full") description of the current version. At a release point, it must be equal to mk_version(). For development builds it may differ. It will indicate the closest in history tag, the number of commits since that tag, and the initial part of the SHA1 of the tip of the current branch.

`mk_openssl_version()` returns the OpenSSL version that we link with.

`mk_libevent_version()` returns the libevent version that we link with.

