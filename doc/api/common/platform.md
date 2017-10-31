# NAME

`measurement_kit/common/platform.h`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_PLATFORM_HPP
#define MEASUREMENT_KIT_COMMON_PLATFORM_HPP

#ifdef __cplusplus
extern "C" {
#endif

const char *mk_platform(void);

#ifdef __cplusplus
} // namespace mk
#endif
#endif
```

# DESCRIPTION

`mk_platform` returns the operating system platform. 

Returns `"android"` on Android. 

Returns `"linux"` on Linux. 

Returns `"windows"` on Windows. 

Returns `"ios"` on iOS. 

Returns `"macos"` on macOS. 

Returns `"unknown"` otherwise.

