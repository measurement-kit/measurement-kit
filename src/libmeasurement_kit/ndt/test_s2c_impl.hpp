// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_TEST_S2C_IMPL_HPP
#define SRC_NDT_TEST_S2C_IMPL_HPP

#include "src/libmeasurement_kit/ndt/internal.hpp"

namespace mk {
namespace ndt {
namespace test_s2c {

using namespace mk::report;

template <MK_MOCK_NAMESPACE(net, connect_many)>
void coroutine_impl(Var<Entry> report_entry, std::string address, Params params,
                    Callback<Error, Continuation<Error, double>> cb,
                    double timeout, Settings settings, Var<Logger> logger,
                    Var<Reactor> reactor) {

    dump_settings(settings, "ndt/s2c", logger);

    // The coroutine connects to the remote endpoint and then pauses
    logger->debug("ndt: connect ...");
    net_connect_many(
        address, params.port, params.num_streams,
        [=](Error err, std::vector<Var<Transport>> txp_list) {
            logger->debug("ndt: connect ... %d", (int)err);
            if (err) {
                cb(err, nullptr);
                return;
            }
            logger->info("Connected to %s:%d", address.c_str(), params.port);
            logger->debug("ndt: suspend coroutine");
            cb(NoError(), [=](Callback<Error, double> cb) {

                // The coroutine is resumed and receives data
                logger->debug("ndt: resume coroutine");
                logger->info("Starting download");
                double begin = time_now();
                Var<size_t> total(new size_t(0));
                Var<double> previous(new double(0.0));
                Var<size_t> count(new size_t(0));
                Var<size_t> num_completed{new size_t{0}};
                size_t num_flows = txp_list.size();
                *previous = begin;
                log_speed(logger, "download-speed", params.num_streams,
                          0.0, 0.0);
                Var<Entry> run_entry(new Entry);
                (*run_entry)["params"]["num_streams"] = params.num_streams;
                (*run_entry)["params"]["snaps_delay"] = params.snaps_delay;
                (*run_entry)["receiver_data"].push_back({0.0, 0.0});

                for (auto txp : txp_list) {
                    txp->set_timeout(timeout);

                    txp->on_data([=](Buffer data) {
                        *total += data.length();
                        double ct = time_now();
                        *count += data.length();
                        if (ct - *previous > params.snaps_delay) {
                            double el = ct - begin;
                            double x = (*count * 8) / 1000 / (ct - *previous);
                            *count = 0;
                            *previous = ct;
                            log_speed(logger, "download-speed",
                                      params.num_streams, el, x);
                            (*run_entry)["receiver_data"].push_back({el, x});
                        }
                        // TODO: force close the connection after a given
                        // large amount of time has passed
                    });

                    txp->on_error([=](Error err) {
                        logger->info("Ending download (%d)", err.code);
                        if (err == EofError()) {
                            err = NoError();
                        }
                        txp->close([=]() {
                            ++(*num_completed);
                            // Note: in this callback we cannot reference
                            // txp_list or txp because that would keep
                            // alive txp indefinitely, so we use the num_flows
                            // variable instead (note that this means that
                            // the `=` only copies what you use, a thing that
                            // I was totally unaware of!)
                            if (*num_completed < num_flows) {
                                return;
                            }
                            double elapsed_time = time_now() - begin;
                            logger->debug("ndt: elapsed %lf", elapsed_time);
                            logger->debug("ndt: total %lu", (unsigned long)*total);
                            double speed = 0.0;
                            if (elapsed_time > 0.0) {
                                speed = *total * 8.0 / 1000.0 / elapsed_time;
                            }
                            logger->info("S2C speed %lf kbit/s", speed);
                            (*report_entry)["s2c_test"].push_back(*run_entry);
                            // XXX We need to define what we consider
                            // error when we have parallel flows
                            cb((num_flows == 1) ? err : NoError(), speed);
                        });
                    });
                }
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
                ctx->logger->debug("%s", e.c_str());
                messages::add_to_report(ctx->entry, "web100_data", e);
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
        ctx->logger->debug("ndt: recv TEST_PREPARE ... %d", err.code);
        if (err) {
            callback(ReadingTestPrepareError(err));
            return;
        }
        if (type != TEST_PREPARE) {
            callback(NotTestPrepareError());
            return;
        }
        Params params;
        std::vector<std::string> vec = split<std::vector<std::string>>(s);
        ErrorOr<int> port = lexical_cast_noexcept<int>(vec[0]);
        if (!port || *port < 0 || *port > 65535) {
            ctx->logger->warn("Received invalid port: %s", vec[0].c_str());
            callback(InvalidPortError());
            return;
        }
        params.port = *port;
        // Here we are being liberal; in theory we should only accept these
        // extra parameters when the test is S2C_EXT
        if (vec.size() >= 2) {
            ErrorOr<double> duration = lexical_cast_noexcept<double>(vec[1]);
            if (!duration or *duration < 0 or *duration > 60000.0) {
                ctx->logger->warn("Received invalid duration: %s",
                                  vec[1].c_str());
                callback(InvalidDurationError());
                return;
            }
            params.duration = *duration / 1000.0;
        }
        ctx->logger->debug("Duration: %f s", params.duration);
        ctx->logger->debug("Snaps-enabled: /* ignored */"); // TODO: implement
        if (vec.size() >= 4) {
            ErrorOr<double> snaps_delay = lexical_cast_noexcept<double>(vec[3]);
            if (!snaps_delay or *snaps_delay < 250.0) {
                ctx->logger->warn("Received invalid snaps-delay: %s",
                                  vec[3].c_str());
                callback(InvalidSnapsDelayError());
                return;
            }
            params.snaps_delay = *snaps_delay / 1000.0;
        }
        ctx->logger->debug("Snaps-delay: %f s", params.snaps_delay);
        ctx->logger->debug("Snaps-offset: /* ignored */"); // TODO: implement
        if (vec.size() >= 6) {
            ErrorOr<int> num_streams = lexical_cast_noexcept<int>(vec[5]);
            if (!num_streams or *num_streams < 1 or *num_streams > 8) {
                ctx->logger->warn("Received invalid num-streams: %s",
                                  vec[5].c_str());
                callback(InvalidNumStreamsError());
                return;
            }
            params.num_streams = *num_streams;
        }
        ctx->logger->debug("Num-streams: %d", params.num_streams);

        // We connect to the port and wait for coroutine to pause
        ctx->logger->debug("ndt: start s2c coroutine ...");
        coroutine(
            ctx->entry, ctx->address, params,
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
