## libmeasurement_kit

file(
  GLOB
  MK_LIBRARY_SOURCES
  "${MK_ROOT}/include/measurement_kit/*.hpp"
  "${MK_ROOT}/include/measurement_kit/*/*.hpp"
  "${MK_ROOT}/src/libmeasurement_kit/*/*.c"
  "${MK_ROOT}/src/libmeasurement_kit/*/*.cpp"
)

add_library(
  measurement_kit_static
  STATIC
  ${MK_LIBRARY_SOURCES}
)
target_include_directories(
  measurement_kit_static
  PUBLIC
  "${MK_ROOT}/include"
  ${MK_INCLUDE_DIRS}
)
target_link_libraries(
  measurement_kit_static
  ${MK_LIBS}
  Threads::Threads
)
