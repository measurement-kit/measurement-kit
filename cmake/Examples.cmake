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
