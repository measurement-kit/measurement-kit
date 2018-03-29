# headers

install(
    DIRECTORY
    ${MK_ROOT}/include/
    DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}
)

# libmeasurement_kit_objects

add_library(
    libmeasurement_kit_objects
    OBJECT
    ${MK_ROOT}/src/libmeasurement_kit/common/citrus_ctype.h
    ${MK_ROOT}/src/libmeasurement_kit/ext/http_parser.h
    ${MK_ROOT}/src/libmeasurement_kit/ext/tls_internal.h
    ${MK_ROOT}/src/libmeasurement_kit/common/citrus_utf8.c
    ${MK_ROOT}/src/libmeasurement_kit/ext/http_parser.c
    ${MK_ROOT}/src/libmeasurement_kit/ext/tls_verify.c
    ${MK_ROOT}/src/libmeasurement_kit/portable/strtonum.c
    ${MK_ROOT}/src/libmeasurement_kit/common/delegate.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/encoding.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/every.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/fapply.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/fcar.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/fcdr.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/fcompose.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/fmap.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/freverse.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/libevent_reactor.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/locked.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/maybe.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/mock.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/parallel.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/range.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/utils.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/worker.hpp
    ${MK_ROOT}/src/libmeasurement_kit/dns/getaddrinfo_async.hpp
    ${MK_ROOT}/src/libmeasurement_kit/dns/libevent_query.hpp
    ${MK_ROOT}/src/libmeasurement_kit/dns/ping.hpp
    ${MK_ROOT}/src/libmeasurement_kit/dns/system_resolver.hpp
    ${MK_ROOT}/src/libmeasurement_kit/dns/utils.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ext/catch.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ext/sole.hpp
    ${MK_ROOT}/src/libmeasurement_kit/http/request_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/http/response_parser.hpp
    ${MK_ROOT}/src/libmeasurement_kit/mlabns/mlabns.hpp
    ${MK_ROOT}/src/libmeasurement_kit/mlabns/mlabns_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/internal.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/measure_speed.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/messages_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/protocol_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/run_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/test_c2s_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/test_meta_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/test_s2c_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/utils.hpp
    ${MK_ROOT}/src/libmeasurement_kit/net/builtin_ca_bundle.hpp
    ${MK_ROOT}/src/libmeasurement_kit/net/connect.hpp
    ${MK_ROOT}/src/libmeasurement_kit/net/connect_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/net/emitter.hpp
    ${MK_ROOT}/src/libmeasurement_kit/net/evbuffer.hpp
    ${MK_ROOT}/src/libmeasurement_kit/net/libevent_emitter.hpp
    ${MK_ROOT}/src/libmeasurement_kit/net/libssl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/net/socks5.hpp
    ${MK_ROOT}/src/libmeasurement_kit/net/utils.hpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/runnable.hpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/utils.hpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/utils_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/neubot/dash_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/bouncer_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/collector_client_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/constants.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/http_header_field_manipulation.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/orchestrate_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/resources_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/utils.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/utils_impl.hpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/whatsapp.hpp
    ${MK_ROOT}/src/libmeasurement_kit/traceroute/android.hpp
    ${MK_ROOT}/src/libmeasurement_kit/traceroute/error.hpp
    ${MK_ROOT}/src/libmeasurement_kit/traceroute/interface.hpp
    ${MK_ROOT}/src/libmeasurement_kit/common/encoding_base64.cpp
    ${MK_ROOT}/src/libmeasurement_kit/common/encoding_utf8.cpp
    ${MK_ROOT}/src/libmeasurement_kit/common/every.cpp
    ${MK_ROOT}/src/libmeasurement_kit/common/json.cpp
    ${MK_ROOT}/src/libmeasurement_kit/common/logger.cpp
    ${MK_ROOT}/src/libmeasurement_kit/common/platform.cpp
    ${MK_ROOT}/src/libmeasurement_kit/common/reactor.cpp
    ${MK_ROOT}/src/libmeasurement_kit/common/utils.cpp
    ${MK_ROOT}/src/libmeasurement_kit/common/version.cpp
    ${MK_ROOT}/src/libmeasurement_kit/common/worker.cpp
    ${MK_ROOT}/src/libmeasurement_kit/dns/query.cpp
    ${MK_ROOT}/src/libmeasurement_kit/dns/query_class.cpp
    ${MK_ROOT}/src/libmeasurement_kit/dns/query_type.cpp
    ${MK_ROOT}/src/libmeasurement_kit/engine.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ext/sole.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ffi.cpp
    ${MK_ROOT}/src/libmeasurement_kit/http/parse_url.cpp
    ${MK_ROOT}/src/libmeasurement_kit/http/request.cpp
    ${MK_ROOT}/src/libmeasurement_kit/http/response_parser.cpp
    ${MK_ROOT}/src/libmeasurement_kit/mlabns/mlabns.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/messages.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/protocol.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/run.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/test_c2s.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/test_meta.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/test_s2c.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ndt/utils.cpp
    ${MK_ROOT}/src/libmeasurement_kit/net/buffer.cpp
    ${MK_ROOT}/src/libmeasurement_kit/net/builtin_ca_bundle.cpp
    ${MK_ROOT}/src/libmeasurement_kit/net/connect.cpp
    ${MK_ROOT}/src/libmeasurement_kit/net/emitter.cpp
    ${MK_ROOT}/src/libmeasurement_kit/net/socks5.cpp
    ${MK_ROOT}/src/libmeasurement_kit/net/transport.cpp
    ${MK_ROOT}/src/libmeasurement_kit/net/utils.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/base_test.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/captive_portal.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/dash.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/dns_injection.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/facebook_messenger.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/http_header_field_manipulation.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/http_invalid_request_line.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/meek_fronted_requests.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/multi_ndt.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/ndt.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/runnable.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/tcp_connect.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/telegram.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/utils.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/web_connectivity.cpp
    ${MK_ROOT}/src/libmeasurement_kit/nettests/whatsapp.cpp
    ${MK_ROOT}/src/libmeasurement_kit/neubot/dash.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/bouncer.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/captive_portal.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/collector_client.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/dns_injection.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/facebook_messenger.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/http_header_field_manipulation.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/http_invalid_request_line.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/meek_fronted_requests.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/orchestrate.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/resources.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/tcp_connect.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/telegram.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/templates.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/utils.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/web_connectivity.cpp
    ${MK_ROOT}/src/libmeasurement_kit/ooni/whatsapp.cpp
    ${MK_ROOT}/src/libmeasurement_kit/report/base_reporter.cpp
    ${MK_ROOT}/src/libmeasurement_kit/report/entry.cpp
    ${MK_ROOT}/src/libmeasurement_kit/report/file_reporter.cpp
    ${MK_ROOT}/src/libmeasurement_kit/report/ooni_reporter.cpp
    ${MK_ROOT}/src/libmeasurement_kit/report/report.cpp
    ${MK_ROOT}/src/libmeasurement_kit/traceroute/android.cpp
    ${MK_ROOT}/src/libmeasurement_kit/traceroute/interface.cpp
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
