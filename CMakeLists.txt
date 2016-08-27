# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

cmake_minimum_required(VERSION 3.6)

set (CMAKE_CXX_STANDARD 11)
file(GLOB sources "src/libmeasurement_kit/*/*.c"
     "src/libmeasurement_kit/*/*.cpp")

add_library(measurement_kit STATIC ${sources})
target_include_directories(
  measurement_kit
  PUBLIC
  /usr/local/Cellar/openssl/1.0.2h_1/include
  ${CMAKE_SOURCE_DIR}/include
)
