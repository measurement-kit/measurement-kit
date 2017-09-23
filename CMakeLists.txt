# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software under the BSD license. See AUTHORS
# and LICENSE for more information on the copying conditions.

project(MeasurementKit)

cmake_minimum_required(VERSION 3.6)

# Definitions:

set(MK_CA_BUNDLE "${MK_CA_BUNDLE}" CACHE PATH "Path where openssl CA bundle is installed")
set(MK_GEOIP "${MK_GEOIP}" CACHE PATH "Path where geoip is installed")
set(MK_LIBEVENT "${MK_LIBEVENT}" CACHE PATH "Path where libevent is installed")
set(MK_OPENSSL "${MK_OPENSSL}" CACHE PATH "Path where openssl is installed")

# Compiler:

set(CMAKE_CXX_STANDARD 14)
set(MK_UNIX_CFLAGS "-Wall -Wextra -pedantic -I${CMAKE_SOURCE_DIR}/include")
set(MK_UNIX_CXXFLAGS "-Wall -Wextra -pedantic -I${CMAKE_SOURCE_DIR}/include")

add_definitions(-DENABLE_INTEGRATION_TESTS -DMK_CA_BUNDLE="${MK_CA_BUNDLE}")

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

if(UNIX)
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
  CHECK_FUNCTION_EXISTS(err HAVE_ERR)
  CHECK_FUNCTION_EXISTS(warn HAVE_WARN)
  CHECK_FUNCTION_EXISTS(warnx HAVE_WARNX)
  CHECK_FUNCTION_EXISTS(getopt HAVE_GETOPT)
  CHECK_FUNCTION_EXISTS(getopt_long HAVE_GETOPT_LONG)
  CHECK_FUNCTION_EXISTS(getopt_long_only HAVE_GETOPT_LONG_ONLY)
  CHECK_FUNCTION_EXISTS(gmtime_r HAVE_GMTIME_R)
  CHECK_FUNCTION_EXISTS(strtonum HAVE_STRTONUM)

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

## libmeasurement_kit

file(
  GLOB
  MK_LIBRARY_SOURCES
  "include/measurement_kit/*.hpp"
  "include/measurement_kit/*/*.hpp"
  "src/libmeasurement_kit/*/*.c"
  "src/libmeasurement_kit/*/*.cpp"
)

add_library(
  measurement_kit_static
  STATIC
  ${MK_LIBRARY_SOURCES}
)
target_include_directories(
  measurement_kit_static
  PUBLIC
  "include"
  ${MK_INCLUDE_DIRS}
)
target_link_libraries(
  measurement_kit_static
  ${MK_LIBS}
  Threads::Threads
)

## measurement_kit

file(
  GLOB
  MK_PROGRAM_SOURCES
  "src/measurement_kit/*.cpp"
  "src/measurement_kit/*/*.cpp"
)

add_executable(
  measurement_kit_exe
  ${MK_PROGRAM_SOURCES}
)
target_link_libraries(
  measurement_kit_exe
  measurement_kit_static
  ${MK_LIBS}
  Threads::Threads
)

## examples

file(GLOB MK_EXAMPLES_SOURCES RELATIVE "${CMAKE_SOURCE_DIR}" "example/*/*.cpp")
foreach(MK_EXAMPLE_SOURCE ${MK_EXAMPLES_SOURCES})
  string(REPLACE ".cpp" "" MK_EXAMPLE_NAME ${MK_EXAMPLE_SOURCE})
  string(REPLACE "/" "_" MK_EXAMPLE_NAME ${MK_EXAMPLE_NAME})
  add_executable(${MK_EXAMPLE_NAME} ${MK_EXAMPLE_SOURCE})
  target_link_libraries(
    ${MK_EXAMPLE_NAME}
    measurement_kit_static
    ${MK_LIBS}
    Threads::Threads
  )
endforeach()

## tests

enable_testing()

file(GLOB MK_TESTS_SOURCES RELATIVE "${CMAKE_SOURCE_DIR}" "test/*/*.cpp")
foreach(MK_TEST_SOURCE ${MK_TESTS_SOURCES})
  string(REPLACE ".cpp" "" MK_TEST_NAME ${MK_TEST_SOURCE})
  string(REPLACE "/" "_" MK_TEST_NAME ${MK_TEST_NAME})
  add_executable(${MK_TEST_NAME} ${MK_TEST_SOURCE})
  target_link_libraries(
    ${MK_TEST_NAME}
    measurement_kit_static
    ${MK_LIBS}
    Threads::Threads
  )
  add_test(${MK_TEST_NAME} ${MK_TEST_NAME})
endforeach()
