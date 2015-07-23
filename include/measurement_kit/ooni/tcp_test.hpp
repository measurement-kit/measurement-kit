// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_TCP_TEST_HPP
#define MEASUREMENT_KIT_OONI_TCP_TEST_HPP

#include <measurement_kit/net/buffer.hpp>
#include <measurement_kit/net/connection.hpp>

#include <measurement_kit/common/emitter.hpp>
#include <measurement_kit/common/pointer.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/log.hpp>

#include <measurement_kit/ooni/net_test.hpp>

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::common;
using namespace measurement_kit::net;

#if 0
class TCPClient : public measurement_kit::common::EmitterVoid,
        public measurement_kit::common::Emitter<std::string>,
        public measurement_kit::common::Emitter<Error> {

    Connection connection;

public:

    using measurement_kit::common::Emitter<std::string>::on;
    using measurement_kit::common::Emitter<std::string>::emit;
    using measurement_kit::common::Emitter<Error>::on;
    using measurement_kit::common::Emitter<Error>::emit;
    using measurement_kit::common::EmitterVoid::on;
    using measurement_kit::common::EmitterVoid::emit;

    TCPClient() {}

    TCPClient(std::string hostname, std::string port) {
        connection = Connection("PF_UNSPEC", hostname.c_str(),
                port.c_str());
        connection.on_error([this](Error error) {
            logger->debug("tcpclient: error event");
            emit("error", error);
        });
        connection.on_connect([this]() {
            logger->debug("tcpclient: connected event");
            if (connection.enable_read() != 0) {
                throw std::runtime_error("Cannot enable read");
            }
            emit("connect");
        });
        connection.on_flush([this]() {
            logger->debug("tcpclient: flush event");
            emit("flush");
        });
        connection.on_data([this](evbuffer *evb) {
            logger->debug("tcpclient: data event");
            auto buffer = Buffer();
            buffer << evb;
            auto string = buffer.read<char>();
            emit("data", std::move(string));
        });
    }

    void write(std::string data) {
        if (connection.puts(data.c_str()) != 0) {
            throw std::runtime_error("Write failed");
        }
    }
};
#endif

typedef SharedPointer<Connection> TCPClient;           /* XXX */

class TCPTest : public ooni::NetTest {
    using ooni::NetTest::NetTest;

public:
    TCPTest(std::string input_filepath_, Settings options_) :
      ooni::NetTest(input_filepath_, options_) {
        test_name = "tcp_test";
        test_version = "0.0.1";
    };

    TCPClient connect(Settings options,
            std::function<void()>&& cb);
};

}}
#endif
