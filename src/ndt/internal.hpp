// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_INTERNAL_HPP
#define SRC_NDT_INTERNAL_HPP

// This implementation targets v3.7.0 of the NDT protocol
// See <https://github.com/ndt-project/ndt/wiki/NDTProtocol>

#include "src/common/utils.hpp"
#include <list>
#include <measurement_kit/common.hpp>
#include <measurement_kit/ext/json.hpp>
#include <measurement_kit/ndt.hpp>
#include <measurement_kit/net.hpp>
#include <string>

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

#define TEST_MID 1
#define TEST_C2S MK_NDT_UPLOAD
#define TEST_S2C MK_NDT_DOWNLOAD
#define TEST_SFW 8
#define TEST_STATUS 16
#define TEST_META 32

#define KICKOFF_MESSAGE "123456 654321"
#define KICKOFF_MESSAGE_SIZE (sizeof(KICKOFF_MESSAGE) - 1)

#define NDT_PORT 3001
#define NDT_TIMEOUT 10.0

// During the handshake we declare to be measurement-kit version such and
// such that is compatible with version v3.7.0 of NDT
#define MSG_NDT_VERSION "v3.7.0 (measurement-kit/" MEASUREMENT_KIT_VERSION ")"

#define TEST_C2S_DURATION 10.0

namespace mk {
namespace ndt {

using json = nlohmann::json;
using namespace mk::net;

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
    Var<Buffer> buff = Buffer::make();
    Callback<Error> callback;
    std::list<std::string> granted_suite;
    Var<Logger> logger = Logger::global();
    int port = NDT_PORT;
    Var<Reactor> reactor = Reactor::global();
    Settings settings;
    // We always set these two tests because they are the bare minimum
    // required to talk to a NDT server. Other sets could be added as flags
    // by the user using the options passed to the run() function.
    int test_suite = TEST_STATUS | TEST_META;
    double timeout = NDT_TIMEOUT;
    Var<Transport> txp;
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

void read_ll(Var<Context> ctx, Callback<Error, uint8_t, std::string> callback,
             Var<Reactor> reactor = Reactor::global());
void read_json(Var<Context> ctx, Callback<Error, uint8_t, json> callback,
               Var<Reactor> reactor = Reactor::global());
void read_msg(Var<Context> ctx, Callback<Error, uint8_t, std::string> callback,
              Var<Reactor> reactor = Reactor::global());

ErrorOr<Buffer> format_msg_extended_login(unsigned char tests);
ErrorOr<Buffer> format_test_msg(std::string s);

void write(Var<Context>, Buffer, Callback<Error>);
void write_noasync(Var<Context>, Buffer);

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

void connect(Var<Context> ctx, Callback<Error> callback);
void send_extended_login(Var<Context> ctx, Callback<Error> callback);
void recv_and_ignore_kickoff(Var<Context> ctx, Callback<Error> callback);
void wait_in_queue(Var<Context> ctx, Callback<Error> callback);
void recv_version(Var<Context> ctx, Callback<Error> callback);
void recv_tests_id(Var<Context> ctx, Callback<Error> callback);
void run_tests(Var<Context> ctx, Callback<Error> callback);
void recv_results_and_logout(Var<Context> ctx, Callback<Error> callback);
void wait_close(Var<Context> ctx, Callback<Error> callback);
void disconnect_and_callback(Var<Context> ctx, Error err);

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

void coroutine(std::string address, int port, double runtime,
               Callback<Error, Continuation<Error>> cb, double timeout = 10.0,
               Settings settings = {}, Var<Logger> logger = Logger::global(),
               Var<Reactor> reactor = Reactor::global());

void run(Var<Context> ctx, Callback<Error> callback);

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

void run(Var<Context> ctx, Callback<Error> callback);

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

void coroutine(std::string address, int port,
               Callback<Error, Continuation<Error, double>> cb,
               double timeout = 10.0, Settings settings = {},
               Var<Logger> logger = Logger::global(),
               Var<Reactor> reactor = Reactor::global());

void finalizing_test(Var<Context> ctx, Callback<Error> callback);

void run(Var<Context> ctx, Callback<Error> callback);

} // namespace test_s2c

} // namespace mk
} // namespace ndt
#endif
