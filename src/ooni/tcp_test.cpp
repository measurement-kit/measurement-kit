#include "ooni/tcp_test.hpp"

using namespace ight::ooni::tcp_test;

void
TCPClient::emit(const std::string event_name) const
{
    auto it = events.find(event_name);
    if (it != events.end()) {
      for (auto event : it->second) {
        event(); 
      }
    }
}

void
TCPClient::emit(const std::string event_name, std::string data) const
{
    auto it = events_sb.find(event_name);
    if (it != events_sb.end()) {
      for (auto event : it->second) {
        event(data);
      }
    }
}

void
TCPClient::on(const std::string event_name, std::function<void()>&& cb)
{
    events[event_name].push_back(std::move(cb));
}

void
TCPClient::on(const std::string event_name, std::function<void(std::string)>&& cb)
{
    events_sb[event_name].push_back(std::move(cb));
}

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
