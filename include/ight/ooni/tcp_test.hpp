#ifndef LIBIGHT_OONI_TCP_TEST_HPP
# define LIBIGHT_OONI_TCP_TEST_HPP

#include <ight/net/buffer.hpp>
#include <ight/net/connection.hpp>

#include <ight/common/emitter.hpp>
#include <ight/common/pointer.hpp>
#include <ight/common/settings.hpp>
#include <ight/common/log.h>

#include <ight/ooni/net_test.hpp>

namespace ight {
namespace ooni {
namespace tcp_test {

using namespace ight::common::pointer;
using namespace ight::net::connection;

#if 0
class TCPClient : public ight::common::emitter::EmitterVoid,
        public ight::common::emitter::Emitter<std::string>,
        public ight::common::emitter::Emitter<IghtError> {

    Connection connection;

public:

    using ight::common::emitter::Emitter<std::string>::on;
    using ight::common::emitter::Emitter<std::string>::emit;
    using ight::common::emitter::Emitter<IghtError>::on;
    using ight::common::emitter::Emitter<IghtError>::emit;
    using ight::common::emitter::EmitterVoid::on;
    using ight::common::emitter::EmitterVoid::emit;

    TCPClient() {}

    TCPClient(std::string hostname, std::string port) {
        connection = Connection("PF_UNSPEC", hostname.c_str(),
                port.c_str());
        connection.on_error([this](IghtError error) {
            ight_debug("tcpclient: error event");
            emit("error", error);
        });
        connection.on_connect([this]() {
            ight_debug("tcpclient: connected event");
            if (connection.enable_read() != 0) {
                throw std::runtime_error("Cannot enable read");
            }
            emit("connect");
        });
        connection.on_flush([this]() {
            ight_debug("tcpclient: flush event");
            emit("flush");
        });
        connection.on_data([this](evbuffer *evb) {
            ight_debug("tcpclient: data event");
            auto buffer = IghtBuffer();
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

class TCPTest : public net_test::NetTest {
    using net_test::NetTest::NetTest;

public:
    TCPTest(std::string input_filepath_, ight::common::Settings options_) : 
      net_test::NetTest(input_filepath_, options_) {
        test_name = "tcp_test";
        test_version = "0.0.1";
    };
    
    TCPClient connect(ight::common::Settings options,
            std::function<void()>&& cb);
};

}}}

#endif
