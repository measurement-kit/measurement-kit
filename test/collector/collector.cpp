// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include <iostream>

#include <measurement_kit/internal/collector/collector.h>

const char *serializedJSON = R"({
  "data_format_version": "0.2.0",
  "input": "torproject.org",
  "measurement_start_time": "2016-06-04 17:53:13",
  "probe_asn": "AS0",
  "probe_cc": "ZZ",
  "probe_ip": "127.0.0.1",
  "software_name": "measurement_kit",
  "software_version": "0.2.0-alpha.1",
  "test_keys": {"failure": null,"received": [],"sent": []},
  "test_name": "tcp_connect",
  "test_runtime": 0.253494024276733,
  "test_start_time": "2016-06-04 17:53:13",
  "test_version": "0.0.1"
})";

TEST_CASE("we can resubmit a measurement") {
  auto request = mk_collector_resubmit_request_new();
  REQUIRE(request != nullptr);
  mk_collector_resubmit_request_set_content(request, serializedJSON);
  mk_collector_resubmit_request_set_ca_bundle_path(request, "cacert.pem");
  auto response = mk_collector_resubmit(request);
  REQUIRE(response != nullptr);
  auto numlogs = mk_collector_resubmit_response_logs_size(response);
  REQUIRE(numlogs > 0);
  for (size_t i = 0; i < numlogs; ++i) {
    auto s = mk_collector_resubmit_response_logs_at(response, i);
    REQUIRE(s != nullptr);
    std::clog << s << std::endl;
  }
  auto content = mk_collector_resubmit_response_content(response);
  REQUIRE(content != nullptr);
  std::clog << content << std::endl;
  REQUIRE(mk_collector_resubmit_response_good(response));
  mk_collector_resubmit_response_delete(response);
  mk_collector_resubmit_request_delete(request);
}
