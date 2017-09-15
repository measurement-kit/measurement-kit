# NAME

`measurement_kit/common/socket.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_SOCKET_HPP
#define MEASUREMENT_KIT_COMMON_SOCKET_HPP

namespace mk {

#ifdef _WIN32
using socket_t = uintptr_t;
#elif DOXYGEN
using socket_t = platform_dependent;
#else
using socket_t = int;
#endif

} // namespace mk
#endif
```

# DESCRIPTION

`socket_t` is a type suitable to contain a system socket.

