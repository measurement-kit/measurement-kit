// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_RUN_IMPL_HPP
#define SRC_NDT_RUN_IMPL_HPP

#include "src/ndt/internal.hpp"
#include <measurement_kit/mlabns.hpp>

namespace mk {
namespace ndt {

using Phase = void (*)(Var<Context>, Callback<Error>);
using Cleanup = void (*)(Var<Context>, Error);

template <Phase connect, Phase send_login, Phase recv_and_ignore_kickoff,
          Phase wait_in_queue, Phase recv_version, Phase recv_tests_id,
          Phase run_tests, Phase recv_results_and_logout, Phase wait_close,
          Cleanup disconnect_and_callback>
void run_with_specific_server_impl(std::string address, int port,
                                   Callback<Error> callback, Settings settings,
                                   Var<Logger> logger, Var<Reactor> reactor) {

    // Note: this implementation is a template because that allows us to
    // easily change the functions implementing each phase of the protocol
    // thus enabling quick experimentation and unit testing.

    Var<Context> ctx(new Context);
    ctx->address = address;
    ctx->callback = callback;
    ctx->logger = logger;
    ctx->reactor = reactor;
    ctx->port = port;
    ctx->settings = settings;

    dump_settings(ctx->settings, "ndt", ctx->logger);

    // The following code implements this sequence diagram:
    // https://raw.githubusercontent.com/wiki/ndt-project/ndt/NDTProtocol.images/ndt_10.png

    connect(ctx, [ctx](Error err) {
        if (err) {
            disconnect_and_callback(ctx, err);
            return;
        }
        send_login(ctx, [ctx](Error err) {
            if (err) {
                disconnect_and_callback(ctx, err);
                return;
            }
            recv_and_ignore_kickoff(ctx, [ctx](Error err) {
                if (err) {
                    disconnect_and_callback(ctx, err);
                    return;
                }
                wait_in_queue(ctx, [ctx](Error err) {
                    if (err) {
                        disconnect_and_callback(ctx, err);
                        return;
                    }
                    recv_version(ctx, [ctx](Error err) {
                        if (err) {
                            disconnect_and_callback(ctx, err);
                            return;
                        }
                        recv_tests_id(ctx, [ctx](Error err) {
                            if (err) {
                                disconnect_and_callback(ctx, err);
                                return;
                            }
                            run_tests(ctx, [ctx](Error err) {
                                if (err) {
                                    disconnect_and_callback(ctx, err);
                                    return;
                                }
                                recv_results_and_logout(ctx, [ctx](Error err) {
                                    if (err) {
                                        disconnect_and_callback(ctx, err);
                                        return;
                                    }
                                    wait_close(ctx, [ctx](Error err) {
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
}

template <MK_MOCK(run_with_specific_server), MK_MOCK_NAMESPACE(mlabns, query)>
void run_impl(Callback<Error> callback, Settings settings, Var<Logger> logger,
         Var<Reactor> reactor) {
    ErrorOr<int> port = settings.get_noexcept<int>("port", 3001);
    if (!port) {
        callback(port.as_error());
        return;
    }
    std::string address = settings.get<std::string>("address", "");
    if (address != "") {
        run_with_specific_server(address, *port, callback, settings,
                                 logger, reactor);
        return;
    }
    mlabns_query("ndt", [=](Error err, mlabns::Reply reply) {
        if (err) {
            callback(err);
            return;
        }
        run_with_specific_server(reply.fqdn, *port, callback, settings,
                                 logger, reactor);
    }, settings, reactor, logger);
}

} // namespace mk
} // namespace ndt
#endif
