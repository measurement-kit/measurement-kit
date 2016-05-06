// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_TEST_C2S_HPP
#define SRC_NDT_TEST_C2S_HPP

#include "src/common/utils.hpp"
#include "src/ndt/context.hpp"
#include "src/ndt/definitions.hpp"
#include "src/ndt/messages.hpp"
#include <measurement_kit/ndt.hpp>
#include <measurement_kit/net.hpp>

namespace mk {
namespace ndt {
namespace tests {

using namespace net;

/// Coroutine that does the real c2s test
void c2s_coroutine(std::string address, int port, double runtime,
                   Callback<Continuation<>> cb, double timeout = 10.0,
                   Logger *logger = Logger::global(),
                   Poller *poller = Poller::global());

/// Testable implementation of c2s_coroutine()
template <MK_MOCK_NAMESPACE(net, connect)>
void c2s_coroutine_impl(std::string address, int port, double runtime,
                        Callback<Continuation<>> cb, double timeout,
                        Logger *logger, Poller *poller) {

    // Performance note: This implementation does some string copies
    // when sending but in localhost testing this does not seem to be
    // a problem when sending against a MK based discard server.
    //
    // Otherwise, a possible improvement would be to have a fixed
    // size buffer and bypass bufferevent's output logic.
    //
    // In my tests (where speed of course depends on the computer you
    // use), I was able to send at over 1.1 GiB/s and similar speeds
    // were achieved using stripped down NDT code from `test_c2s_clt.c`.

    // FIXME: "The NDT Client in the C2S Throughput test MUST use a 8192 Byte
    // buffer to send the packets through the newly opened connection as fast as
    // possible (i.e. without any delays) for 10 seconds. The content of the
    // buffer SHOULD be initialized a single time and sent repeatedly. The
    // contents of the buffer SHOULD avoid repeating content (to avoid
    // any automatic compression mechanisms) and MUST include only US-ASCII
    // printable characters."

    // TODO: in original NDT code there appears to be a maximum number
    // of bytes to be sent (`lht`), then the test is exited

    std::string str(65535, 'a'); // TODO: fill with random data!

    logger->in_progress("ndt: connect");
    connect(address, port,
            [=](Error err, Var<Transport> txp) {
                logger->complete("ndt: connect", err);
                if (err) {
                    cb(err, nullptr);
                    return;
                }
                logger->info("Connected to %s:%d", address.c_str(), port);
                logger->debug("ndt: suspend coroutine");
                cb(NoError(), [=](Callback<> cb) {
                    double begin = time_now();
                    logger->debug("ndt: resume coroutine");
                    logger->info("Starting upload");
                    txp->set_timeout(timeout);
                    txp->on_flush([=]() {
                        if (time_now() - begin > runtime) {
                            logger->info("Elapsed enough time");
                            txp->emit_error(NoError());
                            return;
                        }
                        txp->write(str.data(), str.size());
                    });
                    txp->on_error([=](Error err) {
                        logger->info("Ending upload (%d)", (int)err);
                        txp->close([=]() {
                            logger->info("Connection to %s:%d closed",
                                         address.c_str(), port);
                            cb(err);
                        });
                    });
                    txp->write(str.data(), str.size());
                });
            },
            {}, logger, poller);
}

/// Run the C2S test
void run_test_c2s(Var<Context> ctx, Callback<> callback);

/// Testable implementation of run_test_c2s()
template <MK_MOCK_NAMESPACE(messages, read),
          MK_MOCK(c2s_coroutine)>
void run_test_c2s_impl(Var<Context> ctx, Callback<> callback) {

    // The server sends us the PREPARE message containing the port number
    ctx->logger->in_progress("ndt: recv TEST_PREPARE");
    read(ctx, [=](Error err, uint8_t type, std::string s) {
        ctx->logger->complete("ndt: recv TEST_PREPARE", err);
        if (err) {
            callback(err);
            return;
        }
        if (type != TEST_PREPARE) {
            callback(GenericError());
            return;
        }
        ErrorOr<int> port = lexical_cast_noexcept<int>(s);
        if (!port || *port < 0 || *port > 65535) {
            callback(GenericError());
            return;
        }

        // We connect to the port and wait for coroutine to pause
        ctx->logger->in_progress("ndt: start c2s coroutine");
        c2s_coroutine(
            ctx->address, *port, 10.0,
            [=](Error err, Continuation<> cc) {
                ctx->logger->complete("ndt: start c2s coroutine", err);
                if (err) {
                    callback(err);
                    return;
                }

                // The server sends us the START message to tell we can start
                ctx->logger->in_progress("ndt: recv TEST_START");
                read(ctx, [=](Error err, uint8_t type, std::string) {
                    ctx->logger->complete("ndt: recv TEST_START", err);
                    if (err) {
                        callback(err);
                        return;
                    }
                    if (type != TEST_START) {
                        callback(GenericError());
                        return;
                    }

                    // We resume coroutine and wait for its completion
                    ctx->logger->debug("ndt: resume c2s coroutine");
                    cc([=](Error err) {
                        ctx->logger->debug("ndt: c2s coroutine complete");
                        if (err) {
                            callback(err);
                            return;
                        }

                        // The server sends us MSG containing throughput
                        ctx->logger->in_progress("ndt: recv TEST_MSG");
                        read(ctx, [=](Error err, uint8_t type, std::string s) {
                            ctx->logger->complete("ndt: recv TEST_MSG", err);
                            if (err) {
                                callback(err);
                                return;
                            }
                            if (type != TEST_MSG) {
                                callback(GenericError());
                                return;
                            }
                            ctx->logger->info("C2S speed %s kbit/s", s.c_str());

                            // The server sends us the FINALIZE message
                            ctx->logger->in_progress("ndt: recv TEST_FINALIZE");
                            read(
                                ctx, [=](Error err, uint8_t type, std::string) {
                                    ctx->logger->complete(
                                        "ndt: recv TEST_FINALIZE", err);
                                    if (err) {
                                        callback(err);
                                        return;
                                    }
                                    if (type != TEST_FINALIZE) {
                                        callback(GenericError());
                                        return;
                                    }

                                    // The C2S phase is now finished
                                    callback(NoError());
                                });
                        });
                    });
                });
            },
            ctx->timeout, ctx->logger, ctx->poller);
    });
}

} // namespace tests
} // namespace ndt
} // namespace mk
#endif
