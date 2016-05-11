// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_TEST_META_IMPL_HPP
#define SRC_NDT_TEST_META_IMPL_HPP

#include "src/ndt/internal.hpp"

namespace mk {
namespace ndt {
namespace test_meta {

template <MK_MOCK_NAMESPACE(messages, read),
          MK_MOCK_NAMESPACE(messages, format_test_msg),
          MK_MOCK_NAMESPACE(messages, write)>
void run_impl(Var<Context> ctx, Callback<Error> callback) {

    // The server sends the PREPARE and START messages back to back
    ctx->logger->debug("ndt: recv TEST_PREPARE ...");
    read(ctx, [=](Error err, uint8_t type, std::string) {
        ctx->logger->debug("ndt: recv TEST_PREPARE ... %d", (int)err);
        if (err) {
            callback(err);
            return;
        }
        if (type != TEST_PREPARE) {
            callback(GenericError());
            return;
        }
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

            // Now we send all the TEST messages containing metadata
            ErrorOr<Buffer> out;

            out = format_test_msg("client.version:" MEASUREMENT_KIT_VERSION);
            if (!out) {
                callback(GenericError());
                return;
            }
            messages::write_noasync(ctx, *out);

            out = format_test_msg("client.application:measurement-kit");
            if (!out) {
                callback(GenericError());
                return;
            }
            messages::write_noasync(ctx, *out);

            // XXX not sending: client.os.name
            // XXX not sending: client.browser.name
            // XXX not sending: client.kernel.version

            // Now we send the empty TEST message to signal we're done
            out = format_test_msg("");
            if (!out) {
                callback(GenericError());
                return;
            }

            ctx->logger->debug("ndt: send meta");
            write(ctx, *out, [=](Error err) {
                ctx->logger->debug("ndt: send meta ... %d", (int)err);
                if (err) {
                    callback(err);
                    return;
                }

                ctx->logger->info("Sent additional metadata to server");

                // Now we read the FINALIZE message
                ctx->logger->debug("ndt: recv TEST_FINALIZE ...");
                read(ctx, [=](Error err, uint8_t type, std::string) {
                    ctx->logger->debug("ndt: recv TEST_FINALIZE ... %d", (int)err);
                    if (err) {
                        callback(err);
                        return;
                    }
                    if (type != TEST_FINALIZE) {
                        callback(err);
                        return;
                    }

                    // We're done
                    callback(NoError());
                });
            });
        });
    });
}

} // namespace test_meta
} // namespace mk
} // namespace ndt
#endif
