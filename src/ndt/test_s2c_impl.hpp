// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_TEST_S2C_IMPL_HPP
#define SRC_NDT_TEST_S2C_IMPL_HPP

#include "src/ndt/internal.hpp"

namespace mk {
namespace ndt {
namespace test_s2c {

template <MK_MOCK_NAMESPACE(net, connect)>
void coroutine_impl(std::string address, int port,
                    Callback<Error, Continuation<Error, double>> cb,
                    double timeout, Settings settings, Var<Logger> logger,
                    Var<Reactor> reactor) {

    dump_settings(settings, "ndt/s2c", logger);

    // The coroutine connects to the remote endpoint and then pauses
    logger->debug("ndt: connect ...");
    net_connect(
        address, port,
        [=](Error err, Var<Transport> txp) {
            logger->debug("ndt: connect ... %d", (int)err);
            if (err) {
                cb(err, nullptr);
                return;
            }
            logger->info("Connected to %s:%d", address.c_str(), port);
            logger->debug("ndt: suspend coroutine");
            cb(NoError(), [=](Callback<Error, double> cb) {

                // The coroutine is resumed and receives data
                logger->debug("ndt: resume coroutine");
                logger->info("Starting download");
                Var<double> begin(new double(0.0));
                Var<size_t> total(new size_t(0));
                Var<double> previous(new double(0.0));
                Var<size_t> count(new size_t(0));
                txp->set_timeout(timeout);

                txp->on_data([=](Buffer data) {
                    if (*begin == 0.0) {
                        *begin = *previous = time_now();
                    }
                    *total += data.length();
                    double ct = time_now();
                    *count += data.length();
                    if (ct - *previous > 0.5) {
                        double x = (*count * 8) / 1000 / (ct - *previous);
                        *count = 0;
                        *previous = ct;
                        logger->info("Speed: %.2f kbit/s", x);
                    }
                    // TODO: force close the connection after a given
                    // large amount of time has passed
                });

                txp->on_error([=](Error err) {
                    logger->info("Ending download (%d)", (int)err);
                    double elapsed_time = time_now() - *begin;
                    logger->debug("ndt: elapsed %lf", elapsed_time);
                    logger->debug("ndt: total %lu", (unsigned long)*total);
                    double speed = 0.0;
                    if (err == EofError()) {
                        if (elapsed_time > 0.0) {
                            speed = *total * 8.0 / 1000.0 / elapsed_time;
                        }
                        err = NoError();
                    }
                    logger->info("S2C speed %lf kbit/s", speed);
                    txp->close([=]() { cb(err, speed); });
                });
            });
        },
        settings, logger, reactor);
}

template <MK_MOCK_NAMESPACE(messages, read_msg)>
void finalizing_test_impl(Var<Context> ctx, Callback<Error> callback) {

    ctx->logger->debug("ndt: recv TEST_MSG ...");
    messages_read_msg(ctx, [=](Error err, uint8_t type, std::string s) {
        ctx->logger->debug("ndt: recv TEST_MSG ... %d", (int)err);
        if (err) {
            callback(ReadingTestMsgError(err));
            return;
        }
        if (type == TEST_FINALIZE) {
            callback(NoError());
            return;
        }
        if (type != TEST_MSG) {
            callback(NotTestMsgError());
            return;
        }
        for (auto e : split(s, "\n")) {
            if (e != "") {
                // This should be info because there are Web100
                // variables containing RTT and other useful metrics
                ctx->logger->info("%s", e.c_str());
            }
        }
        // XXX: Here we can loop forever
        finalizing_test_impl<messages_read_msg>(ctx, callback);
    }, ctx->reactor);
}

template <MK_MOCK_NAMESPACE_SUFFIX(messages, read_msg, first),
          MK_MOCK(coroutine),
          MK_MOCK_NAMESPACE_SUFFIX(messages, read_msg, second),
          MK_MOCK_NAMESPACE(messages, read_json),
          MK_MOCK_NAMESPACE(messages, format_test_msg),
          MK_MOCK_NAMESPACE(messages, write), MK_MOCK(finalizing_test)>
void run_impl(Var<Context> ctx, Callback<Error> callback) {

    // The server sends us the PREPARE message containing the port number
    ctx->logger->debug("ndt: recv TEST_PREPARE ...");
    messages_read_msg_first(ctx, [=](Error err, uint8_t type, std::string s) {
        ctx->logger->debug("ndt: recv TEST_PREPARE ... %d", (int)err);
        if (err) {
            callback(ReadingTestPrepareError(err));
            return;
        }
        if (type != TEST_PREPARE) {
            callback(NotTestPrepareError());
            return;
        }
        ErrorOr<int> port = lexical_cast_noexcept<int>(s);
        if (!port || *port < 0 || *port > 65535) {
            callback(InvalidPortError());
            return;
        }

        // We connect to the port and wait for coroutine to pause
        ctx->logger->debug("ndt: start s2c coroutine ...");
        coroutine(
            ctx->address, *port,
            [=](Error err, Continuation<Error, double> cc) {
                ctx->logger->debug("ndt: start s2c coroutine ... %d", (int)err);
                if (err) {
                    callback(ConnectTestConnectionError(err));
                    return;
                }

                // The server sends us the START message to tell we can start
                ctx->logger->debug("ndt: recv TEST_START ...");
                messages_read_msg_second(ctx, [=](Error err, uint8_t type,
                                                  std::string) {
                    ctx->logger->debug("ndt: recv TEST_START ... %d", (int)err);
                    if (err) {
                        callback(ReadingTestStartError(err));
                        return;
                    }
                    if (type != TEST_START) {
                        callback(NotTestStartError());
                        return;
                    }

                    // We resume coroutine and wait for its completion
                    ctx->logger->debug("ndt: resume s2c coroutine");
                    cc([=](Error err, double speed) {
                        ctx->logger->debug("ndt: s2c coroutine complete");
                        if (err) {
                            callback(err);
                            return;
                        }

                        // The server sends us MSG containing throughput
                        ctx->logger->debug("ndt: recv TEST_MSG ...");
                        messages_read_json(ctx, [=](Error err, uint8_t type,
                                                    json m) {
                            ctx->logger->debug("ndt: recv TEST_MSG ... %d",
                                               (int)err);
                            if (err) {
                                callback(ReadingTestMsgError(err));
                                return;
                            }
                            if (type != TEST_MSG) {
                                callback(NotTestMsgError());
                                return;
                            }
                            // TODO: if we will ever parse `m` remember about
                            // possible access errors for json objects
                            ctx->logger->debug("ndt: speed %s",
                                               m.dump().c_str());

                            // We send our measured throughput to the server
                            ctx->logger->debug("ndt: send TEST_MSG ...");
                            ErrorOr<Buffer> out = messages_format_test_msg(
                                lexical_cast<std::string>(speed));
                            if (!out) {
                                callback(SerializingTestMsgError());
                                return;
                            }
                            messages_write(ctx, *out, [=](Error err) {
                                ctx->logger->debug("ndt: send TEST_MSG ... %d",
                                                   (int)err);
                                if (err) {
                                    callback(WritingTestMsgError(err));
                                    return;
                                }

                                // We enter into the final state of this test
                                finalizing_test(ctx, callback);
                            });
                        }, ctx->reactor);
                    });
                }, ctx->reactor);
            },
            ctx->timeout, ctx->settings, ctx->logger, ctx->reactor);
    }, ctx->reactor);
}

} // namespace test_s2c
} // namespace ndt
} // namespace mk
#endif
