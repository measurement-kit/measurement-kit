# Definitions:

if (NOT MK_ROOT)
    set (MK_ROOT "${CMAKE_SOURCE_DIR}")
endif (NOT MK_ROOT)

set(MK_CA_BUNDLE "${MK_CA_BUNDLE}" CACHE PATH "Path where openssl CA bundle is installed")
set(MK_GEOIP "${MK_GEOIP}" CACHE PATH "Path where geoip is installed")
set(MK_LIBEVENT "${MK_LIBEVENT}" CACHE PATH "Path where libevent is installed")
set(MK_OPENSSL "${MK_OPENSSL}" CACHE PATH "Path where openssl is installed")

# Compiler:

set(CMAKE_CXX_STANDARD 14)
set(MK_UNIX_CFLAGS "-Wall -Wextra -pedantic -I${CMAKE_SOURCE_DIR}/include")
set(MK_UNIX_CXXFLAGS "-Wall -Wextra -pedantic -I${CMAKE_SOURCE_DIR}/include")

add_definitions(-DENABLE_INTEGRATION_TESTS -DMK_CA_BUNDLE="${MK_CA_BUNDLE}")
if (WIN32)
    add_definitions(-DNOMINMAX) # https://stackoverflow.com/a/11544154
    add_definitions(
      -D_CRT_SECURE_NO_DEPRECATE) # https://stackoverflow.com/a/14387
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} ${MK_UNIX_CXXFLAGS} -Wmissing-prototypes")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MK_UNIX_CXXFLAGS}")
endif()

if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MK_UNIX_CFLAGS} -Wmissing-prototypes")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MK_UNIX_CFLAGS}")
endif()

# Set target include directories and link libraries:

if(NOT ("${MK_GEOIP}" STREQUAL ""))
    list(APPEND MK_INCLUDE_DIRS "${MK_GEOIP}/include")
    list(APPEND MK_LINK_DIRS "${MK_GEOIP}/lib")
endif()
if(NOT ("${MK_LIBEVENT}" STREQUAL ""))
    list(APPEND MK_INCLUDE_DIRS "${MK_LIBEVENT}/include")
    list(APPEND MK_LINK_DIRS "${MK_LIBEVENT}/lib")
endif()
if(NOT ("${MK_OPENSSL}" STREQUAL ""))
    list(APPEND MK_INCLUDE_DIRS "${MK_OPENSSL}/include")
    list(APPEND MK_LINK_DIRS "${MK_OPENSSL}/lib")
endif()

# Check dependencies:

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)

if(UNIX)

  # libc

  CHECK_FUNCTION_EXISTS(err HAVE_ERR)
  CHECK_FUNCTION_EXISTS(errx HAVE_ERRX)
  CHECK_FUNCTION_EXISTS(warn HAVE_WARN)
  CHECK_FUNCTION_EXISTS(warnx HAVE_WARNX)
  CHECK_FUNCTION_EXISTS(getopt HAVE_GETOPT)
  CHECK_FUNCTION_EXISTS(getopt_long HAVE_GETOPT_LONG)
  CHECK_FUNCTION_EXISTS(getopt_long_only HAVE_GETOPT_LONG_ONLY)
  CHECK_FUNCTION_EXISTS(gmtime_r HAVE_GMTIME_R)
  CHECK_FUNCTION_EXISTS(strtonum HAVE_STRTONUM)

  CHECK_SYMBOL_EXISTS(optreset "getopt.h" HAVE_DECL_OPTRESET)

  # geoip

  set(CMAKE_REQUIRED_INCLUDES "${MK_INCLUDE_DIRS}")
  CHECK_INCLUDE_FILES(GeoIP.h HAVE_GEOIP_H)
  CHECK_LIBRARY_EXISTS(GeoIP GeoIP_open "${MK_GEOIP}/lib" HAVE_LIBGEOIP)
  set(CMAKE_REQUIRED_INCLUDES "")
  if (NOT HAVE_GEOIP_H OR NOT HAVE_LIBGEOIP)
    message(FATAL_ERROR "geoip missing; Use -DMK_GEOIP to specify where it is installed (e.g. -DMK_GEOIP=/usr/local if it is installed under /usr/local)")
  endif()

  # openssl

  set(CMAKE_REQUIRED_INCLUDES "${MK_INCLUDE_DIRS}")
  CHECK_INCLUDE_FILES(openssl/rsa.h HAVE_OPENSSL_RSA_H)
  CHECK_LIBRARY_EXISTS(crypto RSA_new "${MK_OPENSSL}/lib" HAVE_LIBCRYPTO)
  CHECK_INCLUDE_FILES(openssl/ssl.h HAVE_OPENSSL_SSL_H)
  CHECK_LIBRARY_EXISTS(ssl SSL_new "${MK_OPENSSL}/lib" HAVE_LIBSSL)
  set(CMAKE_REQUIRED_INCLUDES "")
  if (NOT HAVE_OPENSSL_RSA_H OR NOT HAVE_LIBSSL)
    message(FATAL_ERROR "openssl/libressl missing; Use -DMK_OPENSSL to specify where it is installed (e.g. -DMK_OPENSSL=/usr/local if it is installed under /usr/local)")
  endif()

  # libevent

  set(CMAKE_REQUIRED_INCLUDES "${MK_INCLUDE_DIRS}")
  CHECK_INCLUDE_FILES(event2/event.h HAVE_EVENT2_EVENT_H)
  CHECK_LIBRARY_EXISTS(event event_new "${MK_LIBEVENT}/lib" HAVE_LIBEVENT)
  set(CMAKE_REQUIRED_INCLUDES "")
  if (NOT HAVE_EVENT2_EVENT_H OR NOT HAVE_LIBEVENT)
    message(FATAL_ERROR "libevent missing; Use -DMK_LIBEVENT to specify where it is installed (e.g. -DMK_LIBEVENT=/usr/local if it is installed under /usr/local)")
  endif()

  # libresolv (required by `./test/common/encoding`)
  CHECK_LIBRARY_EXISTS(resolv hstrerror "" HAVE_LIBRESOLV)
  if (HAVE_LIBRESOLV)
    list(APPEND MK_LIBS resolv)
  endif()

  list(APPEND MK_LIBS GeoIP crypto ssl event event_openssl event_pthreads)
endif()

# Add targets

link_directories(${MK_LINK_DIRS})
