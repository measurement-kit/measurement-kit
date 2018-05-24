# Definitions:

if (NOT MK_ROOT)
  set (MK_ROOT ${CMAKE_SOURCE_DIR})
endif (NOT MK_ROOT)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Set the build type")

set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH
    "Where to install MK")

set(MK_CA_BUNDLE "${MK_CA_BUNDLE}" CACHE PATH
    "Path where openssl CA bundle is installed")

set(MK_GEOIP "${MK_GEOIP}" CACHE PATH
    "Path where geoip is installed")

set(MK_LIBEVENT "${MK_LIBEVENT}" CACHE PATH
    "Path where libevent is installed")

set(MK_OPENSSL "${MK_OPENSSL}" CACHE PATH
    "Path where openssl is installed")

set(MK_BUILD_BINARIES ON CACHE BOOL "Whether to build binaries")

set(MK_BUILD_EXAMPLES ON CACHE BOOL "Whether to build examples")

set(MK_BUILD_TESTS ON CACHE BOOL "Whether to build tests")

set(MK_BUILD_INTEGRATION_TESTS ON CACHE BOOL
    "Whether to build integration tests")

set(MK_DOWNLOAD_DEPS "OFF" CACHE BOOL
    "Whether to download prebuilt dependencies")

# Inclusions:

include(GNUInstallDirs) # CMAKE_INSTALL_<foo>

# Compiler:

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(MK_UNIX_CFLAGS "-Wall -Wextra -pedantic")
set(MK_UNIX_CXXFLAGS "-Wall -Wextra -pedantic")

if (${MK_BUILD_INTEGRATION_TESTS})
  add_definitions(-DENABLE_INTEGRATION_TESTS)
endif()
add_definitions(-DMK_CA_BUNDLE="${MK_CA_BUNDLE}")
if (${WIN32})
  add_definitions(-DNOMINMAX) # https://stackoverflow.com/a/11544154
  add_definitions(
    -D_CRT_SECURE_NO_DEPRECATE) # https://stackoverflow.com/a/14387
  add_definitions(-D_WIN32_WINNT=0x0600) # for inet_ntop()
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_definitions(-DENABLE_TRACEROUTE)
endif()
add_definitions(-DMK_NETTESTS_INTERNAL)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} ${MK_UNIX_CXXFLAGS} -Wmissing-prototypes")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MK_UNIX_CXXFLAGS}")
endif()

if(${CMAKE_C_COMPILER_ID} MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MK_UNIX_CFLAGS} -Wmissing-prototypes")
elseif(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MK_UNIX_CFLAGS}")
endif()

# Download dependencies if needed

include(cmake/utils/DownloadDeps.cmake)
if(${MK_DOWNLOAD_DEPS})
  mk_download_deps("GEOIP;LIBRESSL;LIBEVENT")
endif()

# Set target include directories and link libraries:

list(APPEND CMAKE_REQUIRED_INCLUDES "${MK_ROOT}")
list(APPEND CMAKE_REQUIRED_INCLUDES "${MK_ROOT}/include")

if(NOT ("${MK_GEOIP}" STREQUAL ""))
  list(APPEND CMAKE_REQUIRED_INCLUDES "${MK_GEOIP}/include")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_GEOIP}/lib")
endif()
if(NOT ("${MK_LIBEVENT}" STREQUAL ""))
  list(APPEND CMAKE_REQUIRED_INCLUDES "${MK_LIBEVENT}/include")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_LIBEVENT}/lib")
endif()
if(NOT ("${MK_OPENSSL}" STREQUAL ""))
  list(APPEND CMAKE_REQUIRED_INCLUDES "${MK_OPENSSL}/include")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_OPENSSL}/lib")
endif()

# Check dependencies:

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND MK_LIBS Threads::Threads)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)

CHECK_FUNCTION_EXISTS(err HAVE_ERR)
if(${HAVE_ERR})
  add_definitions(-DHAVE_ERR)
endif()

CHECK_FUNCTION_EXISTS(errx HAVE_ERRX)
if(${HAVE_ERRX})
  add_definitions(-DHAVE_ERRX)
endif()

CHECK_FUNCTION_EXISTS(warn HAVE_WARN)
if(${HAVE_WARN})
  add_definitions(-DHAVE_WARN)
endif()

CHECK_FUNCTION_EXISTS(warnx HAVE_WARNX)
if(${HAVE_WARNX})
  add_definitions(-DHAVE_WARNX)
endif()

CHECK_FUNCTION_EXISTS(getopt HAVE_GETOPT)
if(${HAVE_GETOPT})
  add_definitions(-DHAVE_GETOPT)
endif()

CHECK_FUNCTION_EXISTS(getopt_long HAVE_GETOPT_LONG)
if(${HAVE_GETOPT_LONG})
  add_definitions(-DHAVE_GETOPT_LONG)
endif()

CHECK_FUNCTION_EXISTS(getopt_long_only HAVE_GETOPT_LONG_ONLY)
if(${HAVE_GETOPT_LONG_ONLY})
  add_definitions(-DHAVE_GETOPT_LONG_ONLY)
endif()

CHECK_FUNCTION_EXISTS(gmtime_r HAVE_GMTIME_R)
if(${HAVE_GMTIME_R})
  add_definitions(-DHAVE_GMTIME_R)
endif()

CHECK_FUNCTION_EXISTS(strtonum HAVE_STRTONUM)
if(${HAVE_STRTONUM})
  add_definitions(-DHAVE_STRTONUM)
endif()

CHECK_SYMBOL_EXISTS(optreset "getopt.h" HAVE_DECL_OPTRESET)
if(${HAVE_DECL_OPTRESET})
  add_definitions(-DHAVE_DECL_OPTRESET)
endif()

# Dependencies

## Setup

message(STATUS "CMAKE_REQUIRED_INCLUDES: ${CMAKE_REQUIRED_INCLUDES}")
message(STATUS "CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")

## geoip

CHECK_INCLUDE_FILES(GeoIP.h HAVE_GEOIP_H)
if("${HAVE_GEOIP_H}" STREQUAL "")
  message(FATAL_ERROR "cannot find GeoIP.h")
endif()

FIND_LIBRARY(GEOIP_LIBRARY GeoIP)
if("${GEOIP_LIBRARY}" STREQUAL "GEOIP_LIBRARY-NOTFOUND")
  message(FATAL_ERROR "cannot find GeoIP library")
endif()
message(STATUS "GEOIP_LIBRARY: ${GEOIP_LIBRARY}")
list(APPEND MK_LIBS "${GEOIP_LIBRARY}")

## openssl

CHECK_INCLUDE_FILES(openssl/rsa.h HAVE_OPENSSL_RSA_H)
if("${HAVE_OPENSSL_RSA_H}" STREQUAL "")
  message(FATAL_ERROR "cannot find openssl/rsa.h")
endif()

FIND_LIBRARY(CRYPTO_LIBRARY crypto)
if("${CRYPTO_LIBRARY}" STREQUAL "CRYPTO_LIBRARY-NOTFOUND")
  message(FATAL_ERROR "cannot find crypto library")
endif()
message(STATUS "CRYPTO_LIBRARY: ${CRYPTO_LIBRARY}")
list(APPEND MK_LIBS "${CRYPTO_LIBRARY}")

CHECK_INCLUDE_FILES(openssl/ssl.h HAVE_OPENSSL_SSL_H)
if("${HAVE_OPENSSL_SSL_H}" STREQUAL "")
  message(FATAL_ERROR "cannot find openssl/ssl.h")
endif()

FIND_LIBRARY(SSL_LIBRARY ssl)
if("${SSL_LIBRARY}" STREQUAL "SSL_LIBRARY-NOTFOUND")
  message(FATAL_ERROR "cannot find ssl library")
endif()
message(STATUS "SSL_LIBRARY: ${SSL_LIBRARY}")
list(APPEND MK_LIBS "${SSL_LIBRARY}")

## libevent

CHECK_INCLUDE_FILES(event2/event.h HAVE_EVENT2_EVENT_H)
if("${HAVE_EVENT2_EVENT_H}" STREQUAL "")
  message(FATAL_ERROR "cannot find event2/event.h")
endif()

FIND_LIBRARY(EVENT_LIBRARY event)
if("${EVENT_LIBRARY}" STREQUAL "EVENT_LIBRARY-NOTFOUND")
  message(FATAL_ERROR "cannot find event library")
endif()
message(STATUS "EVENT_LIBRARY: ${EVENT_LIBRARY}")
list(APPEND MK_LIBS "${EVENT_LIBRARY}")

if(${UNIX})
  FIND_LIBRARY(EVENT_OPENSSL_LIBRARY event_openssl)
  if("${EVENT_OPENSSL}" STREQUAL "EVENT_OPENSSL-NOTFOUND")
    message(FATAL_ERROR "cannot find event_openssl library")
  endif()
  message(STATUS "EVENT_OPENSSL_LIBRARY: ${EVENT_OPENSSL_LIBRARY}")
  list(APPEND MK_LIBS "${EVENT_OPENSSL_LIBRARY}")

  FIND_LIBRARY(EVENT_PTHREADS_LIBRARY event_pthreads)
  if("${EVENT_PTHREADS_LIBRARY}" STREQUAL "EVENT_PTHREADS-NOTFOUND")
    message(FATAL_ERROR "cannot find event_pthreads library")
  endif()
  message(STATUS "EVENT_PTHREADS_LIBRARY: ${EVENT_PTHREADS_LIBRARY}")
  list(APPEND MK_LIBS "${EVENT_PTHREADS_LIBRARY}")
endif()

## libresolv (required by `./test/common/encoding`)
CHECK_LIBRARY_EXISTS(resolv hstrerror "" HAVE_LIBRESOLV)
if (HAVE_LIBRESOLV)
  add_definitions(-DHAVE_LIBRESOLV)
  list(APPEND MK_LIBS resolv)
endif()

if(${WIN32})
  list(APPEND MK_LIBS ws2_32)
endif()

# Common rules:

include_directories(${CMAKE_REQUIRED_INCLUDES})
