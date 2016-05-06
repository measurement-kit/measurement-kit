// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_PROTOCOL_HPP
#define SRC_NDT_PROTOCOL_HPP

#include "src/ext/json/src/json.hpp"
#include "src/ndt/context.hpp"
#include "src/ndt/messages.hpp"
#include "src/ndt/test-c2s.hpp"
#include "src/ndt/test-s2c.hpp"
#include "src/ndt/test-meta.hpp"
#include <measurement_kit/ndt.hpp>

namespace mk {
namespace ndt {
namespace protocol {

using namespace net;
using json = nlohmann::json;

/// Connects to the ndt server
void connect(Var<Context> ctx, Callback<> callback);

/// Testable implementation of connect
template <MK_MOCK_NAMESPACE(net, connect)>
void connect_impl(Var<Context> ctx, Callback<> callback) {
    ctx->logger->in_progress("ndt: connect");
    connect(ctx->address, ctx->port,
            [=](Error err, Var<Transport> txp) {
                ctx->logger->complete("ndt: connect", err);
                if (err) {
                    callback(err);
                    return;
                }
                txp->set_timeout(60.0);
                ctx->conn = txp;
                ctx->logger->info("Connected to %s:%d", ctx->address.c_str(),
                                  ctx->port);
                callback(NoError());
            },
            ctx->settings, ctx->logger, ctx->poller);
}

/// Send EXTENDED_LOGIN message to server
void send_extended_login(Var<Context> ctx, Callback<> callback);

/// Testable implementation of send_extended_login()
template <MK_MOCK_NAMESPACE(messages, format_msg_extended_login),
          MK_MOCK_NAMESPACE(messages, write)>
void send_extended_login_impl(Var<Context> ctx, Callback<> callback) {
    ctx->logger->in_progress("ndt: send login");
    ErrorOr<Buffer> out = format_msg_extended_login(ctx->test_suite);
    if (!out) {
        ctx->logger->complete("ndt: send login", out.as_error());
        callback(out.as_error());
        return;
    }
    write(ctx, *out, [=](Error err) {
        ctx->logger->complete("ndt: send login", err);
        if (err) {
            callback(err);
            return;
        }
        ctx->logger->info("Sent LOGIN with test suite: %d", ctx->test_suite);
        callback(NoError());
    });
}

/// Receive and ignore the special kickoff message
void recv_and_ignore_kickoff(Var<Context> ctx, Callback<> callback);

/// Testable implementation of recv_and_ignore_kickoff
template <MK_MOCK_NAMESPACE(net, readn)>
void recv_and_ignore_kickoff_impl(Var<Context> ctx, Callback<> callback) {
    ctx->logger->in_progress("ndt: recv and ignore kickoff");
    readn(ctx->conn, ctx->buff, KICKOFF_MESSAGE_SIZE, [=](Error err) {
        ctx->logger->complete("ndt: recv and ignore kickoff", err);
        if (err) {
            callback(err);
            return;
        }
        std::string s = ctx->buff->readn(KICKOFF_MESSAGE_SIZE);
        if (s != KICKOFF_MESSAGE) {
            callback(GenericError());
            return;
        }
        ctx->logger->info("Got legacy KICKOFF message (ignored)");
        callback(NoError());
    });
}

/// Wait for our turn in queue for some time
void wait_in_queue(Var<Context> ctx, Callback<> callback);

/// Testable implementation of wait_in_queue()
template <MK_MOCK_NAMESPACE(messages, read)>
void wait_in_queue_impl(Var<Context> ctx, Callback<> callback) {
    ctx->logger->in_progress("ndt: wait in queue");
    read(ctx, [=](Error err, uint8_t type, std::string s) {
        ctx->logger->complete("ndt: wait in queue", err);
        if (err) {
            callback(err);
            return;
        }
        if (type != SRV_QUEUE) {
            callback(GenericError());
            return;
        }
        ErrorOr<unsigned> wait_time = lexical_cast_noexcept<unsigned>(s);
        if (!wait_time) {
            callback(wait_time.as_error());
            return;
        }
        ctx->logger->info("Wait time before test starts: %d", *wait_time);
        // XXX Simplified implementation
        if (*wait_time > 0) {
            callback(GenericError());
            return;
        }
        callback(NoError());
    });
}

/// Receive LOGIN message from server with the server's version
void recv_version(Var<Context> ctx, Callback<> callback);

/// Testable implementation of recv_version()
template <MK_MOCK_NAMESPACE(messages, read)>
void recv_version_impl(Var<Context> ctx, Callback<> callback) {
    ctx->logger->in_progress("ndt: recv server version");
    read(ctx, [=](Error err, uint8_t type, std::string s) {
        ctx->logger->complete("ndt: recv server version", err);
        if (err) {
            callback(err);
            return;
        }
        if (type != MSG_LOGIN) {
            callback(GenericError());
            return;
        }
        ctx->logger->info("Got server version: %s", s.c_str());
        // TODO: validate the server version?
        callback(NoError());
    });
}

/// Receive IDs of tests the server is willing to perform with us
void recv_tests_id(Var<Context> ctx, Callback<> callback);

/// Testable implementation of recv_tests_id()
template <MK_MOCK_NAMESPACE(messages, read)>
void recv_tests_id_impl(Var<Context> ctx, Callback<> callback) {
    ctx->logger->in_progress("ndt: recv tests ID");
    read(ctx, [=](Error err, uint8_t type, std::string s) {
        ctx->logger->complete("ndt: recv tests ID", err);
        if (err) {
            callback(err);
            return;
        }
        if (type != MSG_LOGIN) {
            callback(GenericError());
            return;
        }
        ctx->logger->info("Authorized tests: %s", s.c_str());
        // FIXME: honor test suite returned by server
        ctx->granted_suite.push_back(TEST_C2S);
        ctx->granted_suite.push_back(TEST_S2C);
        ctx->granted_suite.push_back(TEST_META);
        callback(NoError());
    });
}

void run_tests(Var<Context> ctx, Callback<> callback); ///< Run tests

/// Testable implementation of run_tests()
template <MK_MOCK_NAMESPACE(messages, read_json),
          MK_MOCK_NAMESPACE(messages, format_test_msg)>
void run_tests_impl(Var<Context> ctx, Callback<> callback) {

    if (ctx->granted_suite.size() <= 0) {
        callback(NoError());
        return;
    }

    int num = ctx->granted_suite.front();
    ctx->granted_suite.pop_front();

    if (num == TEST_C2S) {
        ctx->logger->info("Run C2S test...");
        tests::run_test_c2s(ctx, [=](Error err) {
            ctx->logger->info("Run C2S test... complete (%d)", (int)err);
            if (err) {
                callback(err);
                return;
            }
            run_tests_impl<read_json, format_test_msg>(ctx, callback);
        });
        return;
    }

    if (num == TEST_S2C) {
        ctx->logger->info("Run S2C test...");
        tests::run_test_s2c(ctx, [=](Error err) {
            ctx->logger->info("Run S2C test... complete (%d)", (int)err);
            if (err) {
                callback(err);
                return;
            }
            run_tests_impl<read_json, format_test_msg>(ctx, callback);
        });
        return;
    }

    if (num == TEST_META) {
        ctx->logger->info("Run META test...");
        tests::run_test_meta(ctx, [=](Error err) {
            ctx->logger->info("Run META test... complete (%d)", (int)err);
            if (err) {
                callback(err);
                return;
            }
            run_tests_impl<read_json, format_test_msg>(ctx, callback);
        });
        return;
    }

    ctx->logger->warn("ndt: unknown test: %d", num);
    callback(GenericError());
}

/// Recv results message and then logout message
void recv_results_and_logout(Var<Context> ctx, Callback<> callback);

/// Testable implementation of recv_results_and_logout()
template <MK_MOCK_NAMESPACE(messages, read)>
void recv_results_and_logout_impl(Var<Context> ctx, Callback<> callback) {
    ctx->logger->in_progress("ndt: recv RESULTS");
    read(ctx, [=](Error err, uint8_t type, std::string) {
        ctx->logger->complete("ndt: recv RESULTS", err);
        if (err) {
            callback(err);
            return;
        }
        if (type == MSG_RESULTS) {
            // XXX: here we have the potential to loop forever...
            recv_results_and_logout_impl<read>(ctx, callback);
            return;
        }
        if (type != MSG_LOGOUT) {
            callback(GenericError());
            return;
        }
        ctx->logger->debug("Got LOGOUT");
        callback(NoError());
    });
}

/// Wait for server to close the connection
void wait_close(Var<Context> ctx, Callback<> callback);

/// Testable implementation of wait_close()
template <MK_MOCK_NAMESPACE(net, read)>
void wait_close_impl(Var<Context> ctx, Callback<> callback) {
    ctx->logger->in_progress("ndt: wait close");
    ctx->conn->set_timeout(1.0);
    Var<Buffer> buffer(new Buffer);
    read(ctx->conn, buffer, [=](Error err) {
        ctx->logger->complete("ndt: wait close", err);
        // Note: the server SHOULD close the connection
        if (err == EofError()) {
            ctx->logger->info("Connection closed");
            callback(NoError());
            return;
        }
        if (err == TimeoutError()) {
            ctx->logger->info("Closing connection after 1.0 sec timeout");
            callback(NoError());
            return;
        }
        if (err) {
            callback(err);
            return;
        }
        ctx->logger->debug("ndt: got extra data: %s", buffer->read().c_str());
        callback(GenericError());
    });
}

/// Close connection and invoke final callback
void disconnect_and_callback(Var<Context> ctx, Error err);

} // namespace protocol
} // namespace mk
} // namespace ndt
#endif
