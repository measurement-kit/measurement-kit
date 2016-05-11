// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_CLIENT_IMPL_HPP
#define SRC_NDT_CLIENT_IMPL_HPP

#include "src/ndt/context.hpp"
#include <measurement_kit/common.hpp>
#include <measurement_kit/ndt.hpp>

namespace mk {
namespace ndt {

using Phase = void (*)(Var<Context>, Callback<Error>);
using Cleanup = void (*)(Var<Context>, Error);

template <Phase connect, Phase send_login, Phase recv_and_ignore_kickoff,
          Phase wait_in_queue, Phase recv_version, Phase recv_tests_id,
          Phase run_tests, Phase recv_results_and_logout, Phase wait_close,
          Cleanup disconnect_and_callback>
void client_impl(std::string address, int port, Callback<Error> callback,
                 Settings settings, Var<Logger> logger, Var<Reactor> reactor) {

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

} // namespace mk
} // namespace ndt
#endif
