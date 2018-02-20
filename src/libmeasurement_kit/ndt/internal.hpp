// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NDT_INTERNAL_HPP
#define SRC_LIBMEASUREMENT_KIT_NDT_INTERNAL_HPP

// This implementation targets v3.7.0 of the NDT protocol
// See <https://github.com/ndt-project/ndt/wiki/NDTProtocol>

#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ndt/error.hpp"
#include "src/libmeasurement_kit/ndt/run.hpp"
#include "src/libmeasurement_kit/ndt/measure_speed.hpp"

#include <measurement_kit/net.hpp>
#include <measurement_kit/ndt/defs.hpp>

/*
 ____        __ _       _ _   _
|  _ \  ___ / _(_)_ __ (_) |_(_) ___  _ __  ___
| | | |/ _ \ |_| | '_ \| | __| |/ _ \| '_ \/ __|
| |_| |  __/  _| | | | | | |_| | (_) | | | \__ \
|____/ \___|_| |_|_| |_|_|\__|_|\___/|_| |_|___/

    Definitions used by the NDT test.
*/

#define COMM_FAILURE 0
#define SRV_QUEUE 1
#define MSG_LOGIN 2
#define TEST_PREPARE 3
#define TEST_START 4
#define TEST_MSG 5
#define TEST_FINALIZE 6
#define MSG_ERROR 7
#define MSG_RESULTS 8
#define MSG_LOGOUT 9
#define MSG_WAITING 10
#define MSG_EXTENDED_LOGIN 11

// Those are the original defines of NDT. I'd rather use them in the
// implementation rather than using our define names.
#define TEST_MID MK_NDT_MIDDLEBOX
#define TEST_C2S MK_NDT_UPLOAD
#define TEST_S2C MK_NDT_DOWNLOAD
#define TEST_SFW MK_NDT_SIMPLE_FIREWALL
#define TEST_STATUS MK_NDT_STATUS
#define TEST_META MK_NDT_META
#define TEST_C2S_EXT MK_NDT_UPLOAD_EXT
#define TEST_S2C_EXT MK_NDT_DOWNLOAD_EXT

static inline std::string id_to_name(int id) {
    switch (id) {
    case MK_NDT_MIDDLEBOX:
        return "middlebox";
    case MK_NDT_UPLOAD:
        return "upload";
    case MK_NDT_DOWNLOAD:
        return "download";
    case MK_NDT_SIMPLE_FIREWALL:
        return "simple_firewall";
    case MK_NDT_STATUS:
        return "status";
    case MK_NDT_META:
        return "meta";
    case MK_NDT_UPLOAD_EXT:
        return "upload_ext";
    case MK_NDT_DOWNLOAD_EXT:
        return "download_ext";
    default:
        break;
    }
    return "unknown";
}

#define KICKOFF_MESSAGE "123456 654321"
#define KICKOFF_MESSAGE_SIZE (sizeof(KICKOFF_MESSAGE) - 1)

#define NDT_PORT 3001
#define NDT_TIMEOUT 10.0

// During the handshake we declare to be measurement-kit version such and
// such that is compatible with version v3.7.0 of NDT
#define MSG_NDT_VERSION "v3.7.0"

#define TEST_C2S_DURATION 10.0

#define SRV_QUEUE_HEARTBEAT 9990
#define SRV_QUEUE_SERVER_FAULT 9977
#define SRV_QUEUE_SERVER_BUSY 9987
#define SRV_QUEUE_SERVER_BUSY_60s 9999

namespace mk {
namespace ndt {

using namespace mk::net;
using namespace mk::report;

/*
  ____            _            _
 / ___|___  _ __ | |_ _____  _| |_
| |   / _ \| '_ \| __/ _ \ \/ / __|
| |__| (_) | | | | ||  __/>  <| |_
 \____\___/|_| |_|\__\___/_/\_\\__|

    Data structure representing a NDT test.
*/

struct Context {
    std::string address;
    SharedPtr<Buffer> buff = Buffer::make();
    Callback<Error> callback;
    SharedPtr<Entry> entry;
    std::list<std::string> granted_suite;
    size_t granted_suite_count = 0;
    size_t current_test_count = 0;
    SharedPtr<Logger> logger = Logger::global();
    int port = NDT_PORT;
    SharedPtr<Reactor> reactor = Reactor::global();
    Settings settings;
    // We always set these two tests because they are the bare minimum
    // required to talk to a NDT server. Other sets could be added as flags
    // by the user using the options passed to the run() function.
    int test_suite = TEST_STATUS | TEST_META;
    double timeout = NDT_TIMEOUT;
    SharedPtr<Transport> txp;
};

/*
 __  __
|  \/  | ___  ___ ___  __ _  __ _  ___  ___
| |\/| |/ _ \/ __/ __|/ _` |/ _` |/ _ \/ __|
| |  | |  __/\__ \__ \ (_| | (_| |  __/\__ \
|_|  |_|\___||___/___/\__,_|\__, |\___||___/
                            |___/

    Sending and receiving NDT protocol messages.
*/
namespace messages {

void read_ll(SharedPtr<Context> ctx, Callback<Error, uint8_t, std::string> callback,
             SharedPtr<Reactor> reactor = Reactor::global());
void read_json(SharedPtr<Context> ctx, Callback<Error, uint8_t, Json> callback,
               SharedPtr<Reactor> reactor = Reactor::global());
void read_msg(SharedPtr<Context> ctx, Callback<Error, uint8_t, std::string> callback,
              SharedPtr<Reactor> reactor = Reactor::global());

ErrorOr<Buffer> format_msg_extended_login(unsigned char tests);
ErrorOr<Buffer> format_test_msg(std::string s);
ErrorOr<Buffer> format_msg_waiting();

void write(SharedPtr<Context>, Buffer, Callback<Error>);
void write_noasync(SharedPtr<Context>, Buffer);

Error add_to_report(SharedPtr<Entry> entry, std::string key, std::string item);

} // namespace messages

/*
 ____            _                  _
|  _ \ _ __ ___ | |_ ___   ___ ___ | |
| |_) | '__/ _ \| __/ _ \ / __/ _ \| |
|  __/| | | (_) | || (_) | (_| (_) | |
|_|   |_|  \___/ \__\___/ \___\___/|_|

    Implementation of NDT's control protocol.
*/
namespace protocol {

void connect(SharedPtr<Context> ctx, Callback<Error> callback);
void send_extended_login(SharedPtr<Context> ctx, Callback<Error> callback);
void recv_and_ignore_kickoff(SharedPtr<Context> ctx, Callback<Error> callback);
void wait_in_queue(SharedPtr<Context> ctx, Callback<Error> callback);
void recv_version(SharedPtr<Context> ctx, Callback<Error> callback);
void recv_tests_id(SharedPtr<Context> ctx, Callback<Error> callback);
void run_tests(SharedPtr<Context> ctx, Callback<Error> callback);
void recv_results_and_logout(SharedPtr<Context> ctx, Callback<Error> callback);
void wait_close(SharedPtr<Context> ctx, Callback<Error> callback);
void disconnect_and_callback(SharedPtr<Context> ctx, Error err);

} // namespace protocol

/*
 _____         _      ____ ____  ____
|_   _|__  ___| |_   / ___|___ \/ ___|
  | |/ _ \/ __| __| | |     __) \___ \
  | |  __/\__ \ |_  | |___ / __/ ___) |
  |_|\___||___/\__|  \____|_____|____/

    Client to server test: upload data and measure speed.
*/
namespace test_c2s {

void coroutine(SharedPtr<Entry>, std::string address, int port, double runtime,
               Callback<Error, Continuation<Error>> cb, double timeout = 10.0,
               Settings settings = {}, SharedPtr<Reactor> reactor = Reactor::global(),
               SharedPtr<Logger> logger = Logger::global());

void run(SharedPtr<Context> ctx, Callback<Error> callback);

} // namespace test_c2s

/*
 _____         _     __  __ _____ _____  _
|_   _|__  ___| |_  |  \/  | ____|_   _|/ \
  | |/ _ \/ __| __| | |\/| |  _|   | | / _ \
  | |  __/\__ \ |_  | |  | | |___  | |/ ___ \
  |_|\___||___/\__| |_|  |_|_____| |_/_/   \_\


    META test: send metadata describing the client to the server
*/
namespace test_meta {

void run(SharedPtr<Context> ctx, Callback<Error> callback);

} // namespace test_meta

/*
 _____         _     ____ ____   ____
|_   _|__  ___| |_  / ___|___ \ / ___|
  | |/ _ \/ __| __| \___ \ __) | |
  | |  __/\__ \ |_   ___) / __/| |___
  |_|\___||___/\__| |____/_____|\____|

    Test sever to client: download data and measure speed
*/
namespace test_s2c {

struct Params {
    int port = -1;
    double duration = 10.0;      // ignored by our implementation
    bool snaps_enabled = false;  // we always take snaps, never report them
    double snaps_delay = 0.5;    // we ignore what is sent by the server
    double snaps_offeset = 0.0;  // ignored by our implementation
    int num_streams = 1;

    Params(){}

    // This constructor only sets the port and all the other settings
    // instead remain at their default value
    Params(int port) : port(port) {}
};

void coroutine(SharedPtr<Entry> report_entry, std::string address, Params params,
               Callback<Error, Continuation<Error, double>> cb,
               double timeout = 10.0, Settings settings = {},
               SharedPtr<Reactor> reactor = Reactor::global(),
               SharedPtr<Logger> logger = Logger::global());

void finalizing_test(SharedPtr<Context> ctx, SharedPtr<Entry> cur_entry,
                     Callback<Error> callback);

void run(SharedPtr<Context> ctx, Callback<Error> callback);

} // namespace test_s2c

/*
 _   _ _   _ _
| | | | |_(_) |___
| | | | __| | / __|
| |_| | |_| | \__ \
 \___/ \__|_|_|___/

    Useful functions used by all modules.
*/

inline void log_speed(SharedPtr<Logger> logger, std::string type, int num_streams,
                      double elapsed, double speed) {
    if (speed > 0 && elapsed < 10.0) {
        std::stringstream ss;
        ss << type << " (elapsed " << std::fixed << std::setprecision(2)
           << elapsed << " s) " << std::fixed << std::setprecision(2)
           << speed << " kbit/s " << "(num_streams " << num_streams << ")";
        logger->progress_relative(0.025, ss.str().c_str());
    }
    logger->log(MK_LOG_EVENT | MK_LOG_INFO, R"xx({
            "type": "%s",
            "elapsed": [%lf, "s"],
            "num_streams": %d,
            "speed": [%lf, "kbit/s"]
        })xx", type.c_str(), elapsed, num_streams, speed);
}

} // namespace mk
} // namespace ndt
#endif
