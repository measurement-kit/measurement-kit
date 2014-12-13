#ifndef LIBIGHT_OONI_DNS_TEST_HPP
# define LIBIGHT_OONI_DNS_TEST_HPP

#include "net/connection.h"
#include "common/emitter.hpp"
#include "common/settings.hpp"
#include "ooni/net_test.hpp"

namespace ight {
namespace ooni {
namespace tcp_test {

class TCPClient : public ight::common::emitter::EmitterVoid,
        public ight::common::emitter::Emitter<std::string> {

    using ight::common::emitter::Emitter<std::string>::on;
    using ight::common::emitter::Emitter<std::string>::emit;
    using ight::common::emitter::EmitterVoid::on;
    using ight::common::emitter::EmitterVoid::emit;

public:
    TCPClient() {
        // TODO: implement
    }

    void write(std::string data, std::function<void()>) {
        // TODO: implement
    }
};


class TCPTest : public net_test::NetTest {
    using net_test::NetTest::NetTest;

    IghtConnection connection;

public:
    TCPTest(std::string input_filepath_, ight::common::Settings options_) : 
      net_test::NetTest(input_filepath_, options_) {
        test_name = "tcp_test";
        test_version = "0.0.1";
    };
    
    TCPClient
    connect(ight::common::Settings options, std::function<void()>&& cb);
    
};

}}}

#endif
