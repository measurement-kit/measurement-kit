// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_TEST_C2S_IMPL_HPP
#define SRC_NDT_TEST_C2S_IMPL_HPP

#include "src/ndt/internal.hpp"

namespace mk {
namespace ndt {
namespace test_c2s {

template <MK_MOCK_NAMESPACE(net, connect)>
void coroutine_impl(std::string address, int port, double runtime,
                    Callback<Error, Continuation<Error>> cb, double timeout,
                    Var<Logger> logger, Var<Reactor> reactor) {

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

    std::string str = random_printable(8192);

    logger->debug("ndt: connect ...");
    connect(address, port,
            [=](Error err, Var<Transport> txp) {
                logger->debug("ndt: connect ... %d", (int)err);
                if (err) {
                    cb(err, nullptr);
                    return;
                }
                logger->info("Connected to %s:%d", address.c_str(), port);
                logger->debug("ndt: suspend coroutine");
                cb(NoError(), [=](Callback<Error> cb) {
                    double begin = time_now();
                    Var<double> previous(new double(begin));
                    Var<size_t> count(new size_t(0));
                    logger->debug("ndt: resume coroutine");
                    logger->info("Starting upload");
                    txp->set_timeout(timeout);
                    txp->on_flush([=]() {
                        double now = time_now();
                        if (now - *previous > 0.5) {
                            double x = (*count * 8) / 1000 / (now - *previous);
                            *previous = now;
                            *count = 0;
                            logger->info("Speed: %.2f kbit/s", x);
                        }
                        if (now - begin > runtime) {
                            logger->info("Elapsed enough time");
                            txp->emit_error(NoError());
                            return;
                        }
                        txp->write(str.data(), str.size());
                        *count += str.size();
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
            {}, logger, reactor);
}

template <MK_MOCK_NAMESPACE(messages, read),
          MK_MOCK_NAMESPACE(test_c2s, coroutine)>
void run_impl(Var<Context> ctx, Callback<Error> callback) {

    // The server sends us the PREPARE message containing the port number
    ctx->logger->debug("ndt: recv TEST_PREPARE ...");
    read(ctx, [=](Error err, uint8_t type, std::string s) {
        ctx->logger->debug("ndt: recv TEST_PREPARE ... %d", (int)err);
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
        ctx->logger->debug("ndt: start c2s coroutine ...");
        coroutine(
            ctx->address, *port, 10.0,
            [=](Error err, Continuation<Error> cc) {
                ctx->logger->debug("ndt: start c2s coroutine ... %d", (int)err);
                if (err) {
                    callback(err);
                    return;
                }

                // The server sends us the START message to tell we can start
                ctx->logger->debug("ndt: recv TEST_START ...");
                read(ctx, [=](Error err, uint8_t type, std::string) {
                    ctx->logger->debug("ndt: recv TEST_START ... %d", (int)err);
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
                        ctx->logger->debug("ndt: recv TEST_MSG ...");
                        read(ctx, [=](Error err, uint8_t type, std::string s) {
                            ctx->logger->debug("ndt: recv TEST_MSG ... %d",
                                               (int)err);
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
                            ctx->logger->debug("ndt: recv TEST_FINALIZE ...");
                            read(ctx, [=](Error err, uint8_t type,
                                          std::string) {
                                ctx->logger->debug(
                                    "ndt: recv TEST_FINALIZE ... %d", (int)err);
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
            ctx->timeout, ctx->logger, ctx->reactor);
    });
}

} // namespace test_c2s
} // namespace ndt
} // namespace mk
#endif
