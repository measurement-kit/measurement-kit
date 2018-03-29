# measurement_kit

if (${MK_BUILD_BINARIES})
    add_executable(
        measurement_kit
        ${MK_ROOT}/src/measurement_kit/portable/_getopt.h
        ${MK_ROOT}/src/measurement_kit/portable/err.h
        ${MK_ROOT}/src/measurement_kit/portable/getopt.h
        ${MK_ROOT}/src/measurement_kit/portable/unistd.h
        ${MK_ROOT}/src/measurement_kit/cmdline.hpp
        ${MK_ROOT}/src/measurement_kit/cmd/captive_portal.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/dash.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/dns_injection.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/facebook_messenger.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/http_header_field_manipulation.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/http_invalid_request_line.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/meek_fronted_requests.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/multi_ndt.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/ndt.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/tcp_connect.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/telegram.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/web_connectivity.cpp
        ${MK_ROOT}/src/measurement_kit/cmd/whatsapp.cpp
        ${MK_ROOT}/src/measurement_kit/main.cpp
        ${MK_ROOT}/src/measurement_kit/utils.cpp
    )

    target_link_libraries(
        measurement_kit
        libmeasurement_kit
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
