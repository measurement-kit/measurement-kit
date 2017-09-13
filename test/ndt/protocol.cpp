// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/ndt/protocol_impl.hpp"
#include "private/net/emitter.hpp"

using namespace mk;
using namespace mk::ndt;
using namespace mk::net;
using json = nlohmann::json;

static void fail(std::string, int, Callback<Error, SharedPtr<Transport>> cb, Settings,
                 SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(MockedError(), nullptr);
}

TEST_CASE("we deal with connect() errors") {
    SharedPtr<Context> ctx(new Context);
    protocol::connect_impl<fail>(ctx, [](Error err) {
        REQUIRE(err == ConnectControlConnectionError());
    });
}

static ErrorOr<Buffer> fail(unsigned char) { return MockedError(); }

TEST_CASE("send_extended_login() deals with message formatting error") {
    SharedPtr<Context> ctx(new Context);
    protocol::send_extended_login_impl<fail>(ctx, [](Error err) {
        REQUIRE(err == FormatExtendedLoginMessageError());
    });
}

static ErrorOr<Buffer> success(unsigned char) { return Buffer(); }

static void fail(SharedPtr<Context>, Buffer, Callback<Error> cb) {
    cb(MockedError());
}

TEST_CASE("send_extended_login() deals with write error") {
    SharedPtr<Context> ctx(new Context);
    protocol::send_extended_login_impl<success, fail>(ctx, [](Error err) {
        REQUIRE(err == WriteExtendedLoginMessageError());
    });
}

static void fail(SharedPtr<Transport>, SharedPtr<Buffer>, size_t, Callback<Error> cb,
                 SharedPtr<Reactor> = Reactor::global()) {
    cb(MockedError());
}

TEST_CASE("recv_and_ignore_kickoff() deals with readn() error") {
    SharedPtr<Context> ctx(new Context);
    protocol::recv_and_ignore_kickoff_impl<fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingKickoffMessageError()); });
}

static void invalid(SharedPtr<Transport>, SharedPtr<Buffer> buff, size_t n,
                    Callback<Error> cb, SharedPtr<Reactor> = Reactor::global()) {
    std::string s(n, 'a');
    buff->write(s);
    cb(NoError());
}

TEST_CASE("recv_and_ignore_kickoff() deals with invalid kickoff message") {
    SharedPtr<Context> ctx(new Context);
    protocol::recv_and_ignore_kickoff_impl<invalid>(
        ctx, [](Error err) { REQUIRE(err == InvalidKickoffMessageError()); });
}

static void fail(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                 SharedPtr<Reactor> = Reactor::global()) {
    cb(MockedError(), 0, "");
}

TEST_CASE("wait_in_queue() deals with read() error") {
    SharedPtr<Context> ctx(new Context);
    protocol::wait_in_queue_impl<fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingSrvQueueMessageError()); });
}

static void unexpected(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                       SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), MSG_ERROR, "");
}

TEST_CASE("wait_in_queue() deals with unexpected message error") {
    SharedPtr<Context> ctx(new Context);
    protocol::wait_in_queue_impl<unexpected>(
        ctx, [](Error err) { REQUIRE(err == NotSrvQueueMessageError()); });
}

static void bad_time(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                     SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), SRV_QUEUE, "xo");
}

TEST_CASE("wait_in_queue() deals with invalid wait time") {
    SharedPtr<Context> ctx(new Context);
    protocol::wait_in_queue_impl<bad_time>(
        ctx, [](Error err) { REQUIRE(err == InvalidSrvQueueMessageError()); });
}

static void call_soon_not_called(Callback<> &&, SharedPtr<Reactor>) {
    REQUIRE(false /* should not happen */);
}

static void s_fault(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                    SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), SRV_QUEUE, "9977" /* SRV_QUEUE_SERVER_FAULT */);
}

TEST_CASE("wait_in_queue() deals with server-fault wait time") {
    SharedPtr<Context> ctx(new Context);
    protocol::wait_in_queue_impl<s_fault, messages::format_msg_waiting,
                                 messages::write_noasync, call_soon_not_called>
                                 (ctx, [](Error err) {
        REQUIRE(err == QueueServerFaultError());
    });
}

static void s_busy(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                   SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), SRV_QUEUE, "9987" /* SRV_QUEUE_SERVER_BUSY */);
}

TEST_CASE("wait_in_queue() deals with server-busy wait time") {
    SharedPtr<Context> ctx(new Context);
    protocol::wait_in_queue_impl<s_busy, messages::format_msg_waiting,
                                 messages::write_noasync, call_soon_not_called>(
                                 ctx, [](Error err) {
        REQUIRE(err == QueueServerBusyError());
    });
}

static void s_busy60s(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                      SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), SRV_QUEUE, "9999" /* SRV_QUEUE_SERVER_BUSY_60s */);
}

TEST_CASE("wait_in_queue() deals with server-busy-60s wait time") {
    SharedPtr<Context> ctx(new Context);
    protocol::wait_in_queue_impl<s_busy60s, messages::format_msg_waiting,
                                 messages::write_noasync, call_soon_not_called>(
                                 ctx, [](Error err) {
        REQUIRE(err == QueueServerBusyError());
    });
}

static bool call_soon_called_flag = false;
static void call_soon_called(Callback<> &&, SharedPtr<Reactor>) {
    REQUIRE(!call_soon_called_flag);
    call_soon_called_flag = true;
}

static void heartbeat(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                      SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), SRV_QUEUE, "9990" /* SRV_QUEUE_HEARTBEAT */);
}

static ErrorOr<Buffer> success_format_msg_waiting() {
    return NoError();
}

static bool check_whether_we_write_flag = false;
static void check_whether_we_write(SharedPtr<Context>, Buffer) {
    REQUIRE(!check_whether_we_write_flag);
    check_whether_we_write_flag = true;
}

TEST_CASE("wait_in_queue() deals with heartbeat wait time") {
    SharedPtr<Context> ctx(new Context);
    call_soon_called_flag = false;
    check_whether_we_write_flag = false;
    protocol::wait_in_queue_impl<heartbeat, success_format_msg_waiting,
                                 check_whether_we_write, call_soon_called>(
                                 ctx, [](Error) {
        REQUIRE(false /* should not be called */);
    });
    REQUIRE(check_whether_we_write_flag);
    REQUIRE(call_soon_called_flag);
}

static ErrorOr<Buffer> failure_format_msg_waiting() {
    return MockedError();
}

TEST_CASE("wait_in_queue() deals with format_msg_waiting_error") {
    SharedPtr<Context> ctx(new Context);
    protocol::wait_in_queue_impl<heartbeat, failure_format_msg_waiting,
                                 messages::write_noasync, call_soon_not_called>
                                 (ctx, [](Error err) {
        REQUIRE((err == FormatMsgWaitingError()));
    });
}

static void nonzero(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                    SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), SRV_QUEUE, "1");
}

TEST_CASE("wait_in_queue() deals with nonzero wait time") {
    SharedPtr<Context> ctx(new Context);
    call_soon_called_flag = false;
    protocol::wait_in_queue_impl<nonzero, messages::format_msg_waiting,
                                 messages::write_noasync, call_soon_called>(
                                 ctx, [](Error) {
        REQUIRE(false /* should not be called */);
    });
    REQUIRE(call_soon_called_flag);
}

static void queued_then_whitelisted(SharedPtr<Context>,
        Callback<Error, uint8_t, std::string> cb,
        SharedPtr<Reactor> = Reactor::global()) {
    static int state = 2;
    REQUIRE(state >= 0);
    cb(NoError(), SRV_QUEUE, std::to_string(state).c_str());
    --state;
}

TEST_CASE("wait_in_queue() reschedules itself until we are white listed") {
    SharedPtr<Context> ctx(new Context);
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        protocol::wait_in_queue_impl<queued_then_whitelisted>(ctx, [=](Error e) {
            REQUIRE((e == NoError()));
            reactor->stop();
        });
    });
}

TEST_CASE("recv_version() deals with read() error") {
    SharedPtr<Context> ctx(new Context);
    protocol::recv_version_impl<fail>(ctx, [](Error err) {
        REQUIRE(err == ReadingServerVersionMessageError());
    });
}

TEST_CASE("recv_version() deals with unexpected message error") {
    SharedPtr<Context> ctx(new Context);
    protocol::recv_version_impl<unexpected>(
        ctx, [](Error err) { REQUIRE(err == NotServerVersionMessageError()); });
}

TEST_CASE("recv_tests_id() deals with read() error") {
    SharedPtr<Context> ctx(new Context);
    protocol::recv_tests_id_impl<fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestsIdMessageError()); });
}

TEST_CASE("recv_tests_id() deals with unexpected message error") {
    SharedPtr<Context> ctx(new Context);
    protocol::recv_tests_id_impl<unexpected>(
        ctx, [](Error err) { REQUIRE(err == NotTestsIdMessageError()); });
}

TEST_CASE("run_tests() deals with invalid number") {
    SharedPtr<Context> ctx(new Context);
    ctx->granted_suite.push_front("");
    protocol::run_tests(
        ctx, [](Error err) { REQUIRE(err == InvalidTestIdError()); });
}

TEST_CASE("run_tests() deals with unknown test") {
    SharedPtr<Context> ctx(new Context);
    ctx->granted_suite.push_front("71");
    protocol::run_tests(
        ctx, [](Error err) { REQUIRE(err == UnknownTestIdError()); });
}

static void fail(SharedPtr<Context>, Callback<Error> cb) { cb(MockedError()); }

TEST_CASE("run_tests() deals with test failure") {
    SharedPtr<Context> ctx(new Context);
    ctx->granted_suite.push_front(lexical_cast<std::string>(TEST_C2S));
    ctx->entry = SharedPtr<Entry>::make();
    protocol::run_tests_impl<fail>(ctx, [ctx](Error err) {
        REQUIRE(err == NoError());
        REQUIRE((*ctx->entry)["phase_result"][id_to_name(TEST_C2S)] ==
                "mocked_error");
    });
}

TEST_CASE("recv_results_and_logout() deals with read() error") {
    SharedPtr<Context> ctx(new Context);
    protocol::recv_results_and_logout_impl<fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingResultsOrLogoutError()); });
}

TEST_CASE("recv_results_and_logout() deals with unexpected message error") {
    SharedPtr<Context> ctx(new Context);
    protocol::recv_results_and_logout_impl<unexpected>(
        ctx, [](Error err) { REQUIRE(err == NotResultsOrLogoutError()); });
}

static void eof_error(SharedPtr<Transport>, SharedPtr<Buffer>, Callback<Error> cb,
                      SharedPtr<Reactor> = Reactor::global()) {
    cb(EofError());
}

TEST_CASE("wait_close() deals with EofError") {
    SharedPtr<Context> ctx(new Context);
    ctx->txp.reset(new Emitter(Reactor::global(), Logger::global()));
    protocol::wait_close_impl<eof_error>(
        ctx, [](Error err) { REQUIRE(err == NoError()); });
}

static void timeout_error(SharedPtr<Transport>, SharedPtr<Buffer>, Callback<Error> cb,
                          SharedPtr<Reactor> = Reactor::global()) {
    cb(TimeoutError());
}

TEST_CASE("wait_close() deals with TimeoutError") {
    SharedPtr<Context> ctx(new Context);
    ctx->txp.reset(new Emitter(Reactor::global(), Logger::global()));
    protocol::wait_close_impl<timeout_error>(
        ctx, [](Error err) { REQUIRE(err == NoError()); });
}

static void mocked_error(SharedPtr<Transport>, SharedPtr<Buffer>, Callback<Error> cb,
                         SharedPtr<Reactor> = Reactor::global()) {
    cb(MockedError());
}

TEST_CASE("wait_close() deals with an error") {
    SharedPtr<Context> ctx(new Context);
    ctx->txp.reset(new Emitter(Reactor::global(), Logger::global()));
    protocol::wait_close_impl<mocked_error>(
        ctx, [](Error err) { REQUIRE(err == WaitingCloseError()); });
}

static void no_error(SharedPtr<Transport>, SharedPtr<Buffer>, Callback<Error> cb,
                     SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError());
}

TEST_CASE("wait_close() deals with a extra data") {
    SharedPtr<Context> ctx(new Context);
    ctx->txp.reset(new Emitter(Reactor::global(), Logger::global()));
    protocol::wait_close_impl<no_error>(
        ctx, [](Error err) { REQUIRE(err == DataAfterLogoutError()); });
}

// To increase coverage
TEST_CASE("disconnect_and_callback_impl() without transport") {
    SharedPtr<Context> ctx(new Context);
    ctx->callback = [](Error e) { REQUIRE(e == MockedError()); };
    protocol::disconnect_and_callback_impl(ctx, MockedError());
}

// To increase coverage
TEST_CASE("disconnect_and_callback_impl() with transport") {
    SharedPtr<Context> ctx(new Context);
    ctx->txp.reset(new Emitter(Reactor::global(), Logger::global()));
    ctx->callback = [](Error e) { REQUIRE(e == MockedError()); };
    protocol::disconnect_and_callback_impl(ctx, MockedError());
}
