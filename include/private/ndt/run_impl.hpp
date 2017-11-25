// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NDT_RUN_IMPL_HPP
#define PRIVATE_NDT_RUN_IMPL_HPP

#include "private/common/mock.hpp"

#include "../ndt/internal.hpp"
#include <measurement_kit/mlabns.hpp>

namespace mk {
namespace ndt {

using Phase = void (*)(SharedPtr<Context>, Callback<Error>);
using Cleanup = void (*)(SharedPtr<Context>, Error);

template <Phase connect, Phase send_login, Phase recv_and_ignore_kickoff,
          Phase wait_in_queue, Phase recv_version, Phase recv_tests_id,
          Phase run_tests, Phase recv_results_and_logout, Phase wait_close,
          Cleanup disconnect_and_callback>
void run_with_specific_server_impl(SharedPtr<Entry> entry, std::string address,
                                   int port, Callback<Error> callback,
                                   Settings settings, SharedPtr<Reactor> reactor,
                                   SharedPtr<Logger> logger) {

    // Note: this implementation is a template because that allows us to
    // easily change the functions implementing each phase of the protocol
    // thus enabling quick experimentation and unit testing.

    SharedPtr<Context> ctx{std::make_shared<Context>()};
    ctx->address = address;
    ctx->callback = callback;
    ctx->entry = entry;
    ctx->logger = logger;
    ctx->reactor = reactor;
    ctx->port = port;
    ctx->settings = settings;

    // If the user has not configured the test_suite to run, default with
    // running the download and the uploade phases of the test.
    ctx->test_suite |= settings.get("test_suite", TEST_C2S | TEST_S2C);

    dump_settings(ctx->settings, "ndt", ctx->logger);

    // Initialize entry keys that may be set by this routine
    (*ctx->entry)["client_resolver"] = nullptr; /* Set later by parent */
    (*ctx->entry)["failure"] = nullptr;
    (*ctx->entry)["server_address"] = address;
    (*ctx->entry)["server_port"] = port;
    (*ctx->entry)["server_version"] = nullptr;
    (*ctx->entry)["summary_data"] = Entry::object();
    (*ctx->entry)["test_c2s"] = Entry::array();
    (*ctx->entry)["test_s2c"] = Entry::array();
    (*ctx->entry)["test_suite"] = ctx->test_suite;

#define TRAP_ERRORS(e)                                                         \
    if (e) {                                                                   \
        disconnect_and_callback(ctx, e);                                       \
        return;                                                                \
    }

    // The following code implements this sequence diagram:
    // https://raw.githubusercontent.com/wiki/ndt-project/ndt/NDTProtocol.images/ndt_10.png

    connect(ctx, [ctx](Error err) {
        TRAP_ERRORS(err);
        ctx->logger->progress_relative(0.01, "Connected to test server");

        send_login(ctx, [ctx](Error err) {
            TRAP_ERRORS(err);
            ctx->logger->progress_relative(0.01, "Logged in with test server");

            recv_and_ignore_kickoff(ctx, [ctx](Error err) {
                TRAP_ERRORS(err);
                ctx->logger->progress_relative(0.01,
                                               "Waiting for our turn in queue");

                wait_in_queue(ctx, [ctx](Error err) {
                    TRAP_ERRORS(err);
                    ctx->logger->progress_relative(0.01,
                                                   "Authorized to run test");

                    recv_version(ctx, [ctx](Error err) {
                        TRAP_ERRORS(err);
                        ctx->logger->progress_relative(0.01,
                                                       "Got server version");

                        recv_tests_id(ctx, [ctx](Error err) {
                            TRAP_ERRORS(err);
                            ctx->logger->progress_relative(
                                  0.01, "Got authorized tests identifiers");

                            run_tests(ctx, [ctx](Error err) {
                                TRAP_ERRORS(err);
                                // Progress printed by run_tests()

                                recv_results_and_logout(ctx, [ctx](Error err) {
                                    TRAP_ERRORS(err);
                                    ctx->logger->progress_relative(
                                          0.01, "Received results from server");

                                    wait_close(ctx, [ctx](Error err) {
                                        ctx->logger->progress_relative(
                                              0.01,
                                              "Connection with server closed");
                                        disconnect_and_callback(ctx, err);
                                    });
                                });
                            });
                        });
                    });
                });
            });
        });
    });

#undef TRAP_ERRORS
}

template <MK_MOCK(run_with_specific_server),
          MK_MOCK_AS(mlabns::query, mlabns_query)>
void run_impl(SharedPtr<Entry> entry, Callback<Error> callback, Settings settings,
              SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    ErrorOr<int> port = settings.get_noexcept<int>("port", NDT_PORT);
    if (!port) {
        callback(InvalidPortError(port.as_error()));
        return;
    }
    std::string address = settings.get<std::string>("address", "");
    if (address != "") {
        run_with_specific_server(entry, address, *port, callback, settings,
                                 reactor, logger);
        return;
    }
    mlabns_query(settings.get<std::string>("mlabns_tool_name", "ndt"),
                 [=](Error err, mlabns::Reply reply) {
                     if (err) {
                         callback(MlabnsQueryError(err));
                         return;
                     }
                     run_with_specific_server(entry, reply.fqdn, *port,
                                              callback, settings, reactor,
                                              logger);
                 },
                 settings, reactor, logger);
}

} // namespace ndt
} // namespace mk
#endif
