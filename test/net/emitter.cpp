// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/net/emitter.hpp"
#include <measurement_kit/net.hpp>

using namespace mk;
using namespace mk::net;

TEST_CASE("The record-received-data feature works") {
    SECTION("By default recording is disabled") {
        Emitter emitter;
        Transport &transport = emitter;
        transport.emit_data(Buffer("foobar"));
        REQUIRE(transport.received_data().length() == 0);
    }

    SECTION("Recording works correctly when enabled") {
        Emitter emitter;
        Transport &transport = emitter;
        transport.record_received_data();
        transport.emit_data(Buffer("foobar"));
        REQUIRE(transport.received_data().peek() == "foobar");
    }

    SECTION("Recording can also be disabled") {
        Emitter emitter;
        Transport &transport = emitter;
        transport.record_received_data();
        transport.emit_data(Buffer("foobar"));
        transport.dont_record_received_data();
        transport.emit_data(Buffer("baz"));
        REQUIRE(transport.received_data().peek() == "foobar");
    }

    SECTION("Recording input data does not modify input data") {
        Emitter emitter;
        Transport &transport = emitter;
        transport.record_received_data();
        transport.on_data([](Buffer data) { REQUIRE(data.read() == "foo"); });
        transport.emit_data(Buffer("foo"));
        REQUIRE(transport.received_data().read() == "foo");
    }
}

class Helper : public Emitter {
  public:
    void do_send(Buffer data) override { REQUIRE(data.read() == "foo"); }
};

TEST_CASE("The record-sent-data feature works") {
    SECTION("By default recording is disabled") {
        Emitter emitter;
        Transport &transport = emitter;
        transport.write("foobar");
        REQUIRE(transport.sent_data().length() == 0);
    }

    SECTION("Recording works correctly when enabled") {
        Emitter emitter;
        Transport &transport = emitter;
        transport.record_sent_data();
        transport.write("foobar");
        REQUIRE(transport.sent_data().peek() == "foobar");
    }

    SECTION("Recording can also be disabled") {
        Emitter emitter;
        Transport &transport = emitter;
        transport.record_sent_data();
        transport.write("foobar");
        transport.dont_record_sent_data();
        transport.write("baz");
        REQUIRE(transport.sent_data().peek() == "foobar");
    }

    SECTION("Recording output data does not modify output data") {
        Helper emitter;
        Transport &transport = emitter;
        transport.record_sent_data();
        transport.write("foo");
        REQUIRE(transport.sent_data().read() == "foo");
    }
}
