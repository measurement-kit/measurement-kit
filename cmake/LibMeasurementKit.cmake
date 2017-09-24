## libmeasurement_kit

file(
  GLOB
  MK_LIBRARY_SOURCES
  "${MEASUREMENTKIT_ROOT}/include/measurement_kit/*.hpp"
  "${MEASUREMENTKIT_ROOT}/include/measurement_kit/*/*.hpp"
  "${MEASUREMENTKIT_ROOT}/src/libmeasurement_kit/*/*.c"
  "${MEASUREMENTKIT_ROOT}/src/libmeasurement_kit/*/*.cpp"
)

add_library(
  measurement_kit_static
  STATIC
  ${MK_LIBRARY_SOURCES}
)
target_include_directories(
  measurement_kit_static
  PUBLIC
  "${MEASUREMENTKIT_ROOT}/include"
  ${MK_INCLUDE_DIRS}
)
target_link_libraries(
  measurement_kit_static
  ${MK_LIBS}
  Threads::Threads
)
