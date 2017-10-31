// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/net/emitter.hpp"
#include "private/ooni/collector_client_impl.hpp"

#include <sstream>

using namespace mk::http;
using namespace mk::net;
using namespace mk::ooni;
using namespace mk::report;
using namespace mk;

/*
       _   _ _
 _   _| |_(_) |
| | | | __| | |
| |_| | |_| | |
 \__,_|\__|_|_|

*/

class MockConnection : public Emitter, public NonCopyable, public NonMovable {
    // TODO: consider whether it would make sense to add
    // this functionality directly to Emitter

  public:
    static SharedPtr<Transport> make(SharedPtr<Reactor> reactor) {
        MockConnection *conn = new MockConnection{reactor};
        conn->self = SharedPtr<Transport>(conn);
        return conn->self;
    }

    ~MockConnection() override {
        if (close_cb) {
            close_cb();
        }
    }

    void close(Callback<> cb) override;

  private:
    bool isclosed = false;
    Callback<> close_cb;
    SharedPtr<Transport> self;

    MockConnection(SharedPtr<Reactor> reactor) : Emitter(reactor, Logger::global()) {}
};

void MockConnection::close(Callback<> cb) {
    REQUIRE(!isclosed);
    isclosed = true;
    close_cb = cb;
    reactor->call_soon([=]() {
        self = nullptr;
    });
}

/*
             _ _
 _   _ _ __ (_) |_
| | | | '_ \| | __|
| |_| | | | | | |_
 \__,_|_| |_|_|\__|

*/

TEST_CASE("collector:post deals with missing URL") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::post_impl(nullptr, "", "",
                         [=](Error err, Json) {
                             REQUIRE(err == MissingCollectorBaseUrlError());
                             reactor->stop();
                         },
                         {}, reactor, Logger::global());
    });
}

static void fail(SharedPtr<Transport>, Settings, Headers, std::string,
                 Callback<Error, SharedPtr<Response>> cb, SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(MockedError(), nullptr);
}

const static Settings SETTINGS = {
    {"collector_base_url", collector::testing_collector_url()},
};

TEST_CASE("collector::post deals with network error") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::post_impl<fail>(nullptr, "", "",
                               [=](Error err, Json r) {
                                   REQUIRE(err == MockedError());
                                   REQUIRE(r == nullptr);
                                   reactor->stop();
                               },
                               SETTINGS, reactor, Logger::global());
    });
}

static void five_hundred(SharedPtr<Transport>, Settings, Headers, std::string,
                         Callback<Error, SharedPtr<Response>> cb, SharedPtr<Reactor>,
                         SharedPtr<Logger>) {
    SharedPtr<Response> resp(new Response);
    resp->status_code = 500;
    cb(NoError(), resp);
}

TEST_CASE("collector::post deals with unexpected response") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::post_impl<five_hundred>(
        nullptr, "", "",
        [=](Error err, Json r) {
            REQUIRE(err == HttpRequestFailedError());
            REQUIRE(r == nullptr);
            reactor->stop();
        },
        SETTINGS, reactor, Logger::global());
    });
}

static void empty(SharedPtr<Transport>, Settings, Headers, std::string,
                  Callback<Error, SharedPtr<Response>> cb, SharedPtr<Reactor>,
                  SharedPtr<Logger>) {
    SharedPtr<Response> resp(new Response);
    resp->status_code = 200;
    cb(NoError(), resp);
}

TEST_CASE("collector::post deals with empty response") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::post_impl<empty>(nullptr, "", "",
                                [=](Error err, Json r) {
                                    REQUIRE(err == NoError());
                                    REQUIRE(r == nullptr);
                                    reactor->stop();
                                },
                                SETTINGS, reactor, Logger::global());
    });
}

static void invalid_json(SharedPtr<Transport>, Settings, Headers, std::string,
                         Callback<Error, SharedPtr<Response>> cb, SharedPtr<Reactor>,
                         SharedPtr<Logger>) {
    SharedPtr<Response> resp(new Response);
    resp->status_code = 200;
    resp->body = "{";
    cb(NoError(), resp);
}

TEST_CASE("collector::post deals with invalid json") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::post_impl<invalid_json>(nullptr, "", "",
                                       [=](Error err, Json r) {
                                           REQUIRE(err == JsonParseError());
                                           REQUIRE(r == nullptr);
                                           reactor->stop();
                                       },
                                       SETTINGS, reactor,
                                       Logger::global());
    });
}

TEST_CASE("collector::connect deals with missing URL") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::connect_impl({},
                            [=](Error err, SharedPtr<Transport>) {
                                REQUIRE(err == MissingCollectorBaseUrlError());
                                reactor->stop();
                            },
                            reactor, Logger::global());
    });
}

static void fail(SharedPtr<Transport>, std::string, std::string,
                 Callback<Error, Json> cb, Settings, SharedPtr<Reactor>,
                 SharedPtr<Logger>) {
    cb(MockedError(), nullptr);
}

static Entry ENTRY{
    {"data_format_version", "0.2.0"},
    {"input", "torproject.org"},
    {"measurement_start_time", "2016-06-04 17:53:13"},
    {"probe_asn", "AS0"},
    {"probe_cc", "ZZ"},
    {"probe_ip", "127.0.0.1"},
    {"software_name", "measurement_kit"},
    {"software_version", "0.2.0-alpha.1+11.1"},
    {"test_keys", {
        {"failure", nullptr},
        {"received", Json::array()},
        {"sent", Json::array()},
    }},
    {"test_name", "tcp_connect"},
    {"test_runtime", 0.253494024276733},
    {"test_start_time", "2016-06-04 17:53:13"},
    {"test_version","0.0.1"},
    {"input_hashes", {
        "37e60e13536f6afe47a830bfb6b371b5cf65da66d7ad65137344679b24fdccd1",
        "e0611ecd28bead38a7afeb4dda8ae3449d0fc2e1ba53fa7355f2799dce9af290"
    }},
};

static Entry BAD_ENTRY{
    {"data_format_version", "0.2.0"},
    {"input", "torproject.org"},
    {"measurement_start_time", "2016-06-04 17:53:13"},
    {"probe_asn", "AS0"},
    {"probe_cc", "ZZ"},
    {"probe_ip", "127.0.0.1"},
    {"software_name", "measurement kit"}, // This should fail
    {"software_version", "0.2.0-alpha.1+11.1"},
    {"test_keys", {
        {"failure", nullptr},
        {"received", Json::array()},
        {"sent", Json::array()},
    }},
    {"test_name", "tcp_connect"},
    {"test_runtime", 0.253494024276733},
    {"test_start_time", "2016-06-04 17:53:13"},
    {"test_version","0.0.1"},
    {"input_hashes", {
        "37e60e13536f6afe47a830bfb6b371b5cf65da66d7ad65137344679b24fdccd1",
        "e0611ecd28bead38a7afeb4dda8ae3449d0fc2e1ba53fa7355f2799dce9af290"
    }},
};

TEST_CASE("collector::create_report deals with entry with missing key") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        Entry entry;
        collector::create_report_impl<fail>(
        nullptr, entry,
        [=](Error err, std::string s) {
            REQUIRE(err == MissingMandatoryKeyError());
            REQUIRE(s == "");
            reactor->stop();
        },
        {}, reactor, Logger::global());
    });
}

TEST_CASE("collector::create_report deals with entry with invalid value") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::create_report_impl<fail>(
        nullptr, BAD_ENTRY,
        [=](Error err, std::string s) {
            REQUIRE(err == InvalidMandatoryValueError());
            REQUIRE(s == "");
            reactor->stop();
        },
        {}, reactor, Logger::global());
    });
}

TEST_CASE("collector::create_report deals with POST error") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::create_report_impl<fail>(nullptr, ENTRY,
                                        [=](Error err, std::string s) {
                                            REQUIRE(err == MockedError());
                                            REQUIRE(s == "");
                                            reactor->stop();
                                        },
                                        {}, reactor,
                                        Logger::global());
    });
}

static void wrong_json_type(SharedPtr<Transport>, std::string, std::string,
                            Callback<Error, Json> cb, Settings,
                            SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(NoError(), 17.0);
}

TEST_CASE("collector::create_report deals with wrong JSON type") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::create_report_impl<wrong_json_type>(
        nullptr, ENTRY,
        [=](Error err, std::string s) {
            REQUIRE(err == JsonDomainError());
            REQUIRE(s == "");
            reactor->stop();
        },
        {}, reactor, Logger::global());
    });
}

static void missing_report_id(SharedPtr<Transport>, std::string, std::string,
                              Callback<Error, Json> cb, Settings,
                              SharedPtr<Reactor>, SharedPtr<Logger>) {
    Json json{{"foo", "bar"}, {"bar", "baz"}};
    cb(NoError(), json);
}

TEST_CASE("collector::create_report deals with missing report_id") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::create_report_impl<missing_report_id>(
        nullptr, ENTRY,
        [=](Error err, std::string s) {
            REQUIRE(err == JsonKeyError());
            REQUIRE(s == "");
            reactor->stop();
        },
        {}, reactor, Logger::global());
    });
}

TEST_CASE("collector::update_report deals with entry with missing key") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        Entry entry;
        collector::update_report_impl<fail>(
        nullptr, "xx", entry,
        [=](Error err) {
            REQUIRE(err == MissingMandatoryKeyError());
            reactor->stop();
        },
        {}, reactor, Logger::global());
    });
}

TEST_CASE("collector::get_next_entry() works correctly at EOF") {
    SharedPtr<std::istream> input(new std::istringstream(""));
    ErrorOr<Entry> entry = collector::get_next_entry(input, Logger::global());
    REQUIRE(!entry);
    REQUIRE(entry.as_error() == FileEofError());
}

TEST_CASE("collector::get_next_entry() works correctly on I/O error") {
    SharedPtr<std::istream> input(new std::istringstream(""));
    input->setstate(input->badbit);
    ErrorOr<Entry> entry = collector::get_next_entry(input, Logger::global());
    REQUIRE(!entry);
    REQUIRE(entry.as_error() == FileIoError());
}

TEST_CASE("collector::get_next_entry() works correctly on invalid JSON") {
    SharedPtr<std::istream> input(new std::istringstream("{\n"));
    ErrorOr<Entry> entry = collector::get_next_entry(input, Logger::global());
    REQUIRE(!entry);
    REQUIRE(entry.as_error() == JsonParseError());
}

TEST_CASE("collector::get_next_entry() works correctly on incomplete line") {
    SharedPtr<std::istream> input(new std::istringstream("{}"));
    ErrorOr<Entry> entry = collector::get_next_entry(input, Logger::global());
    REQUIRE(!entry);
    REQUIRE(entry.as_error() == FileEofError());
}

static void fail(SharedPtr<Transport>, std::string, Entry, Callback<Error> cb,
                 Settings, SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(MockedError());
}

TEST_CASE("update_and_fetch_next() deals with update_report error") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::update_and_fetch_next_impl<fail>(
            nullptr, MockConnection::make(reactor), "", 1, {},
            [=](Error err) {
                REQUIRE(err == MockedError());
                reactor->stop();
            },
            {}, reactor, Logger::global());
    });
}

static void success(SharedPtr<Transport>, std::string, Entry, Callback<Error> cb,
                    Settings, SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(NoError());
}

static ErrorOr<Entry> fail(SharedPtr<std::istream>, SharedPtr<Logger>) {
    return MockedError();
}

TEST_CASE("update_and_fetch_next() deals with get_next_entry error") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::update_and_fetch_next_impl<success, fail>(
            nullptr, MockConnection::make(reactor), "", 1, {},
            [=](Error err) {
                REQUIRE(err == MockedError());
                reactor->stop();
            },
            {}, reactor, Logger::global());
    });
}

TEST_CASE("submit_report() deals with invalid file") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::submit_report_impl(
            "/nonexistent/nonexistent-very-long-filename.txt", "", "",
            [=](Error err) {
                REQUIRE(err == CannotOpenReportError());
                reactor->stop();
            },
            {}, reactor, Logger::global());
    });
}

TEST_CASE("submit_report() deals with get_next_entry error") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::submit_report_impl<fail>("test/fixtures/hosts.txt", "", "",
                                            [=](Error err) {
                                                REQUIRE(err == MockedError());
                                                reactor->stop();
                                            },
                                            {}, reactor,
                                            Logger::global());
    });
}

static ErrorOr<Entry> success(SharedPtr<std::istream>, SharedPtr<Logger>) {
    return Entry{};
}

static void fail(Settings, Callback<Error, SharedPtr<Transport>> cb, SharedPtr<Reactor>,
                 SharedPtr<Logger>) {
    cb(MockedError(), nullptr);
}

TEST_CASE("submit_report() deals with collector_connect error") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::submit_report_impl<success, fail>(
            "test/fixtures/hosts.txt", "", "",
            [=](Error err) {
                REQUIRE(err == MockedError());
                reactor->stop();
            },
            {}, reactor, Logger::global());
    });
}

static void success(Settings, Callback<Error, SharedPtr<Transport>> cb,
                    SharedPtr<Reactor> reactor, SharedPtr<Logger>) {
    cb(NoError(), MockConnection::make(reactor));
}

static void fail(SharedPtr<Transport>, Entry, Callback<Error, std::string> cb,
                 Settings, SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(MockedError(), "");
}

TEST_CASE("submit_report() deals with collector_create_report error") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::submit_report_impl<success, success, fail>(
            "test/fixtures/hosts.txt", "", "",
            [=](Error err) {
                REQUIRE(err == MockedError());
                reactor->stop();
            },
            {}, reactor, Logger::global());
    });
}

/*
 _       _                       _   _
(_)_ __ | |_ ___  __ _ _ __ __ _| |_(_) ___  _ __
| | '_ \| __/ _ \/ _` | '__/ _` | __| |/ _ \| '_ \
| | | | | ||  __/ (_| | | | (_| | |_| | (_) | | | |
|_|_| |_|\__\___|\__, |_|  \__,_|\__|_|\___/|_| |_|
                 |___/
*/

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("The collector client works as expected") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        collector::submit_report("test/fixtures/report.njson",
                                 collector::testing_collector_url(),
                                 [=](Error err) {
                                     REQUIRE(err == NoError());
                                     reactor->stop();
                                 }, {}, reactor);
    });
}

#endif
