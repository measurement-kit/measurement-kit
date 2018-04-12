# headers

install(
    DIRECTORY
    ${MK_ROOT}/include/
    DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}
)

# libmeasurement_kit_objects

file(STRINGS "cmake/LibSources.txt" MK_LIB_SOURCES)

add_library(
    libmeasurement_kit_objects
    OBJECT
    ${MK_LIB_SOURCES}
)

set_target_properties(
    libmeasurement_kit_objects
    PROPERTIES
    POSITION_INDEPENDENT_CODE
    ON
)

target_compile_definitions(
    libmeasurement_kit_objects
    PUBLIC
    -DMK_BUILD_DLL
)

# libmeasurement_kit_static

add_library(
    libmeasurement_kit_static
    STATIC
    $<TARGET_OBJECTS:libmeasurement_kit_objects>
)

set_target_properties(
    libmeasurement_kit_static
    PROPERTIES
    OUTPUT_NAME
    "measurement_kit_static"
)

install(
    TARGETS
    libmeasurement_kit_static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib
)

# libmeasurement_kit

add_library(
    libmeasurement_kit
    SHARED
    $<TARGET_OBJECTS:libmeasurement_kit_objects>
)

set_target_properties(
    libmeasurement_kit
    PROPERTIES
    OUTPUT_NAME
    "measurement_kit"
)

target_link_libraries(
    libmeasurement_kit
    ${MK_LIBS}
)

install(
    TARGETS
    libmeasurement_kit
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin
)
