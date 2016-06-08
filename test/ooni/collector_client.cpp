// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include "src/net/emitter.hpp"
#include "src/ooni/collector_client_impl.hpp"
#include <measurement_kit/ooni.hpp>
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
    static Var<Transport> make() {
        MockConnection *conn = new MockConnection;
        conn->self = Var<Transport>(conn);
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
    Var<Reactor> reactor = Reactor::global();
    Var<Transport> self;

    MockConnection() {}
};

void MockConnection::close(Callback<> cb) {
    if (isclosed) {
        throw std::runtime_error("already closed");
    }
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
    collector::post_impl(nullptr, "", "",
                         [=](Error err, nlohmann::json) {
                             REQUIRE(err == MissingCollectorBaseUrlError());
                         },
                         {}, Reactor::global(), Logger::global());
}

static void fail(Var<Transport>, Settings, Headers, std::string,
                 Callback<Error, Var<Response>> cb, Var<Reactor>, Var<Logger>) {
    cb(MockedError(), nullptr);
}

const static Settings SETTINGS = {
    {"collector_base_url", collector::default_collector_url()},
};

TEST_CASE("collector::post deals with network error") {
    collector::post_impl<fail>(nullptr, "", "",
                               [=](Error err, nlohmann::json r) {
                                   REQUIRE(err == MockedError());
                                   REQUIRE(r == nullptr);
                               },
                               SETTINGS, Reactor::global(), Logger::global());
}

static void five_hundred(Var<Transport>, Settings, Headers, std::string,
                         Callback<Error, Var<Response>> cb, Var<Reactor>,
                         Var<Logger>) {
    Var<Response> resp(new Response);
    resp->status_code = 500;
    cb(NoError(), resp);
}

TEST_CASE("collector::post deals with unexpected response") {
    collector::post_impl<five_hundred>(
        nullptr, "", "",
        [=](Error err, nlohmann::json r) {
            REQUIRE(err == HttpRequestFailedError());
            REQUIRE(r == nullptr);
        },
        SETTINGS, Reactor::global(), Logger::global());
}

static void empty(Var<Transport>, Settings, Headers, std::string,
                  Callback<Error, Var<Response>> cb, Var<Reactor>,
                  Var<Logger>) {
    Var<Response> resp(new Response);
    resp->status_code = 200;
    cb(NoError(), resp);
}

TEST_CASE("collector::post deals with empty response") {
    collector::post_impl<empty>(nullptr, "", "",
                                [=](Error err, nlohmann::json r) {
                                    REQUIRE(err == NoError());
                                    REQUIRE(r == nullptr);
                                },
                                SETTINGS, Reactor::global(), Logger::global());
}

static void invalid_json(Var<Transport>, Settings, Headers, std::string,
                         Callback<Error, Var<Response>> cb, Var<Reactor>,
                         Var<Logger>) {
    Var<Response> resp(new Response);
    resp->status_code = 200;
    resp->body = "{";
    cb(NoError(), resp);
}

TEST_CASE("collector::post deals with invalid json") {
    collector::post_impl<invalid_json>(nullptr, "", "",
                                       [=](Error err, nlohmann::json r) {
                                           REQUIRE(err == JsonParseError());
                                           REQUIRE(r == nullptr);
                                       },
                                       SETTINGS, Reactor::global(),
                                       Logger::global());
}

TEST_CASE("collector:connect deals with missing URL") {
    collector::connect_impl({},
                            [=](Error err, Var<Transport>) {
                                REQUIRE(err == MissingCollectorBaseUrlError());
                            },
                            Reactor::global(), Logger::global());
}

static void fail(Var<Transport>, std::string, std::string,
                 Callback<Error, nlohmann::json> cb, Settings, Var<Reactor>,
                 Var<Logger>) {
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
    {"software_version", "0.2.0-alpha.1"},
    {"test_keys", {
        {"failure", nullptr},
        {"received", nlohmann::json::array()},
        {"sent", nlohmann::json::array()},
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
    {"software_version", "0.2.0-alpha.1"},
    {"test_keys", {
        {"failure", nullptr},
        {"received", nlohmann::json::array()},
        {"sent", nlohmann::json::array()},
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
    Entry entry;
    collector::create_report_impl<fail>(
        nullptr, entry,
        [=](Error err, std::string s) {
            REQUIRE(err == MissingMandatoryKeyError());
            REQUIRE(s == "");
        },
        {}, Reactor::global(), Logger::global());
}

TEST_CASE("collector::create_report deals with entry with invalid value") {
    collector::create_report_impl<fail>(
        nullptr, BAD_ENTRY,
        [=](Error err, std::string s) {
            REQUIRE(err == InvalidMandatoryValueError());
            REQUIRE(s == "");
        },
        {}, Reactor::global(), Logger::global());
}

TEST_CASE("collector::create_report deals with POST error") {
    collector::create_report_impl<fail>(nullptr, ENTRY,
                                        [=](Error err, std::string s) {
                                            REQUIRE(err == MockedError());
                                            REQUIRE(s == "");
                                        },
                                        {}, Reactor::global(),
                                        Logger::global());
}

static void wrong_json_type(Var<Transport>, std::string, std::string,
                            Callback<Error, nlohmann::json> cb, Settings,
                            Var<Reactor>, Var<Logger>) {
    cb(NoError(), 17.0);
}

TEST_CASE("collector::create_report deals with wrong JSON type") {
    collector::create_report_impl<wrong_json_type>(
        nullptr, ENTRY,
        [=](Error err, std::string s) {
            REQUIRE(err == JsonDomainError());
            REQUIRE(s == "");
        },
        {}, Reactor::global(), Logger::global());
}

static void missing_report_id(Var<Transport>, std::string, std::string,
                              Callback<Error, nlohmann::json> cb, Settings,
                              Var<Reactor>, Var<Logger>) {
    nlohmann::json json{{"foo", "bar"}, {"bar", "baz"}};
    cb(NoError(), json);
}

TEST_CASE("collector::create_report deals with missing report_id") {
    collector::create_report_impl<missing_report_id>(
        nullptr, ENTRY,
        [=](Error err, std::string s) {
            REQUIRE(err == JsonKeyError());
            REQUIRE(s == "");
        },
        {}, Reactor::global(), Logger::global());
}

TEST_CASE("collector::update_report deals with entry with missing key") {
    Entry entry;
    collector::update_report_impl<fail>(
        nullptr, "xx", entry,
        [=](Error err) {
            REQUIRE(err == MissingMandatoryKeyError());
        },
        {}, Reactor::global(), Logger::global());
}

TEST_CASE("collector::get_next_entry() works correctly at EOF") {
    Var<std::istream> input(new std::istringstream(""));
    ErrorOr<Entry> entry = collector::get_next_entry(input, Logger::global());
    REQUIRE(!entry);
    REQUIRE(entry.as_error() == FileEofError());
}

TEST_CASE("collector::get_next_entry() works correctly on I/O error") {
    Var<std::istream> input(new std::istringstream(""));
    input->setstate(input->badbit);
    ErrorOr<Entry> entry = collector::get_next_entry(input, Logger::global());
    REQUIRE(!entry);
    REQUIRE(entry.as_error() == FileIoError());
}

TEST_CASE("collector::get_next_entry() works correctly on invalid JSON") {
    Var<std::istream> input(new std::istringstream("{\n"));
    ErrorOr<Entry> entry = collector::get_next_entry(input, Logger::global());
    REQUIRE(!entry);
    REQUIRE(entry.as_error() == JsonParseError());
}

TEST_CASE("collector::get_next_entry() works correctly on incomplete line") {
    Var<std::istream> input(new std::istringstream("{}"));
    ErrorOr<Entry> entry = collector::get_next_entry(input, Logger::global());
    REQUIRE(!entry);
    REQUIRE(entry.as_error() == FileEofError());
}

static void fail(Var<Transport>, std::string, Entry, Callback<Error> cb,
                 Settings, Var<Reactor>, Var<Logger>) {
    cb(MockedError());
}

TEST_CASE("update_and_fetch_next() deals with update_report error") {
    loop_with_initial_event([=]() {
        collector::update_and_fetch_next_impl<fail>(
            nullptr, MockConnection::make(), "", 1, {},
            [=](Error err) {
                REQUIRE(err == MockedError());
                break_loop();
            },
            {}, Reactor::global(), Logger::global());
    });
}

static void success(Var<Transport>, std::string, Entry, Callback<Error> cb,
                    Settings, Var<Reactor>, Var<Logger>) {
    cb(NoError());
}

static ErrorOr<Entry> fail(Var<std::istream>, Var<Logger>) {
    return MockedError();
}

TEST_CASE("update_and_fetch_next() deals with get_next_entry error") {
    loop_with_initial_event([=]() {
        collector::update_and_fetch_next_impl<success, fail>(
            nullptr, MockConnection::make(), "", 1, {},
            [=](Error err) {
                REQUIRE(err == MockedError());
                break_loop();
            },
            {}, Reactor::global(), Logger::global());
    });
}

TEST_CASE("submit_report() deals with invalid file") {
    loop_with_initial_event([=]() {
        collector::submit_report_impl(
            "/nonexistent/nonexistent-very-long-filename.txt", "",
            [=](Error err) {
                REQUIRE(err == CannotOpenReportError());
                break_loop();
            },
            {}, Reactor::global(), Logger::global());
    });
}

TEST_CASE("submit_report() deals with get_next_entry error") {
    loop_with_initial_event([=]() {
        collector::submit_report_impl<fail>("test/fixtures/hosts.txt", "",
                                            [=](Error err) {
                                                REQUIRE(err == MockedError());
                                                break_loop();
                                            },
                                            {}, Reactor::global(),
                                            Logger::global());
    });
}

static ErrorOr<Entry> success(Var<std::istream>, Var<Logger>) {
    return Entry{};
}

static void fail(Settings, Callback<Error, Var<Transport>> cb, Var<Reactor>,
                 Var<Logger>) {
    cb(MockedError(), nullptr);
}

TEST_CASE("submit_report() deals with collector_connect error") {
    loop_with_initial_event([=]() {
        collector::submit_report_impl<success, fail>(
            "test/fixtures/hosts.txt", "",
            [=](Error err) {
                REQUIRE(err == MockedError());
                break_loop();
            },
            {}, Reactor::global(), Logger::global());
    });
}

static void success(Settings, Callback<Error, Var<Transport>> cb, Var<Reactor>,
                    Var<Logger>) {
    cb(NoError(), MockConnection::make());
}

static void fail(Var<Transport>, Entry, Callback<Error, std::string> cb,
                 Settings, Var<Reactor>, Var<Logger>) {
    cb(MockedError(), "");
}

TEST_CASE("submit_report() deals with collector_create_report error") {
    loop_with_initial_event([=]() {
        collector::submit_report_impl<success, success, fail>(
            "test/fixtures/hosts.txt", "",
            [=](Error err) {
                REQUIRE(err == MockedError());
                break_loop();
            },
            {}, Reactor::global(), Logger::global());
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

TEST_CASE("The collector client works as expected") {
    loop_with_initial_event([=]() {
        collector::submit_report("test/fixtures/report.json",
                                 collector::default_collector_url(),
                                 [=](Error err) {
                                     REQUIRE(err == NoError());
                                     break_loop();
                                 });
    });
}
