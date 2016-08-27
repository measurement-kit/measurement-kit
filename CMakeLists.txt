# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

cmake_minimum_required(VERSION 3.6)

# Note: CMake is mainly meant for usage under Windows, while under Unix we
# prefer to use autotools; hence assume for Unix this file has quirks.

set(CMAKE_CXX_STANDARD 11)
file(
     GLOB
     sources
     "src/libmeasurement_kit/*/*.c"
     "src/libmeasurement_kit/*/*.cpp"
    )

if(APPLE)
  set(openssl_include_path /usr/local/Cellar/openssl/1.0.2h_1/include)
endif()

add_library(measurement_kit STATIC ${sources})
target_include_directories(
                           measurement_kit
                           PUBLIC
                           ${openssl_include_path}
                           ${CMAKE_SOURCE_DIR}/include
                          )
