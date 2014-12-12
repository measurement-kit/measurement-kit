#ifndef LIBIGHT_OONI_DNS_TEST_HPP
# define LIBIGHT_OONI_DNS_TEST_HPP

#include "net/connection.h"
#include "common/settings.hpp"
#include "ooni/net_test.hpp"

namespace ight {
namespace ooni {
namespace tcp_test {

class TCPTest;

class TCPClient {
TCPTest* tcp_test;

// XXX there is probably a way to make the argument to the function be a
// template.
// See:
// http://stackoverflow.com/questions/14784441/how-to-have-template-type-deduced-in-stdfunction-arguments-with-lambda
// http://stackoverflow.com/questions/13753801/stdfunction-template-argument-resolution
std::map<std::string,
         std::vector<std::function<void()>>> events;

std::map<std::string,
         std::vector<std::function<void(std::stringbuf)>>> events_sb;

public:
    TCPClient(TCPTest* tcp_test_) : tcp_test(tcp_test_) {};

    void
    emit(const std::string event_name) const;

    void
    emit(const std::string event_name, std::stringbuf data) const;
    
    void
    on(const std::string event_name, std::function<void()>&&);

    void
    on(const std::string event_name, std::function<void(std::stringbuf)>&&);

    void
    write(std::stringbuf data, std::function<void()>&&);

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
