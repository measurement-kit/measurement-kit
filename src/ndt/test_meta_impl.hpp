// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_TEST_META_IMPL_HPP
#define SRC_NDT_TEST_META_IMPL_HPP

#include "src/ndt/internal.hpp"

namespace mk {
namespace ndt {
namespace test_meta {

template <MK_MOCK_NAMESPACE_SUFFIX(messages, read_msg, first),
          MK_MOCK_NAMESPACE_SUFFIX(messages, read_msg, second),
          MK_MOCK_NAMESPACE_SUFFIX(messages, format_test_msg, first),
          MK_MOCK_NAMESPACE_SUFFIX(messages, format_test_msg, second),
          MK_MOCK_NAMESPACE_SUFFIX(messages, format_test_msg, third),
          MK_MOCK_NAMESPACE(messages, write),
          MK_MOCK_NAMESPACE_SUFFIX(messages, read_msg, third)>
void run_impl(Var<Context> ctx, Callback<Error> callback) {

    // The server sends the PREPARE and START messages back to back
    ctx->logger->debug("ndt: recv TEST_PREPARE ...");
    messages_read_msg_first(ctx, [=](Error err, uint8_t type, std::string) {
        ctx->logger->debug("ndt: recv TEST_PREPARE ... %d", (int)err);
        if (err) {
            callback(ReadingTestPrepareError(err));
            return;
        }
        if (type != TEST_PREPARE) {
            callback(NotTestPrepareError());
            return;
        }
        ctx->logger->debug("ndt: recv TEST_START ...");
        messages_read_msg_second(
            ctx, [=](Error err, uint8_t type, std::string) {
                ctx->logger->debug("ndt: recv TEST_START ... %d", (int)err);
                if (err) {
                    callback(ReadingTestStartError(err));
                    return;
                }
                if (type != TEST_START) {
                    callback(NotTestStartError());
                    return;
                }

                // Now we send all the TEST messages containing metadata
                ErrorOr<Buffer> out;

                ctx->logger->debug("send client.version");
                out = messages_format_test_msg_first(
                    "client.version:" MEASUREMENT_KIT_VERSION);
                if (!out) {
                    callback(SerializingClientVersionError());
                    return;
                }
                messages::write_noasync(ctx, *out);
                ctx->logger->debug("send client.version ... 0");

                ctx->logger->debug("send client.application");
                out = messages_format_test_msg_second(
                    "client.application:measurement-kit");
                if (!out) {
                    callback(SerializingClientApplicationError());
                    return;
                }
                messages::write_noasync(ctx, *out);
                ctx->logger->debug("send client.application ... 0");

                // XXX not sending: client.os.name
                // XXX not sending: client.browser.name
                // XXX not sending: client.kernel.version

                // Now we send the empty TEST message to signal we're done
                ctx->logger->debug("ndt: send final empty message");
                out = messages_format_test_msg_third("");
                if (!out) {
                    callback(SerializingFinalMetaError());
                    return;
                }
                messages_write(ctx, *out, [=](Error err) {
                    ctx->logger->debug("ndt: send final empty message ... %d",
                                       (int)err);
                    if (err) {
                        callback(WritingMetaError(err));
                        return;
                    }

                    ctx->logger->info("Sent additional metadata to server");

                    // Now we read the FINALIZE message
                    ctx->logger->debug("ndt: recv TEST_FINALIZE ...");
                    messages_read_msg_third(
                        ctx, [=](Error err, uint8_t type, std::string) {
                            ctx->logger->debug("ndt: recv TEST_FINALIZE ... %d",
                                               (int)err);
                            if (err) {
                                callback(ReadingTestFinalizeError(err));
                                return;
                            }
                            if (type != TEST_FINALIZE) {
                                callback(NotTestFinalizeError());
                                return;
                            }

                            // We're done
                            callback(NoError());
                        }, ctx->reactor);
                });
            }, ctx->reactor);
    }, ctx->reactor);
}

} // namespace test_meta
} // namespace mk
} // namespace ndt
#endif
