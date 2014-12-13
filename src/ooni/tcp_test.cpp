#include "ooni/tcp_test.hpp"

using namespace ight::ooni::tcp_test;

TCPClient
TCPTest::connect(ight::common::Settings options, std::function<void()>&& cb) {
    if (options["port"] == "") {
      throw std::runtime_error("Port is required");
    }
    if (options["host"] == "") {
      options["host"] = "localhost";
    }

    TCPClient tcp_client(this);

    connection = IghtConnection("PF_UNSPEC", options["host"].c_str(),
                                options["port"].c_str());
    connection.on_connect([=](){
      entry["connection"] = "success";
      tcp_client.emit("connection");
      cb();
    });
	  connection.on_error([=](IghtError&& e) {
      entry["error_code"] = e.error;
      entry["connection"] = "failed";
      cb();
    });
    return tcp_client;
};
