## tests

enable_testing()

file(GLOB MK_TESTS_SOURCES "${CMAKE_SOURCE_DIR}"
     "${MK_ROOT}/test/*/*.cpp")
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
