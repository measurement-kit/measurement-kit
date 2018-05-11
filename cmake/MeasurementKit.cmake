# measurement_kit

if (${MK_BUILD_BINARIES})
    file(STRINGS "cmake/BinSources.txt" MK_BIN_SOURCES)
    add_executable(
        measurement_kit
        ${MK_BIN_SOURCES}
    )

    target_link_libraries(
        measurement_kit
        libmeasurement_kit_static
        ${MK_LIBS}
    )

    target_compile_definitions(
        measurement_kit
        PUBLIC
        -DMK_USE_DLL
    )

    install(
        TARGETS
        measurement_kit
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin
    )

endif (${MK_BUILD_BINARIES})
