// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "measurement_kit/mkapi/orchestra.h"

#include <iostream>

#include "private/catch.hpp"

TEST_CASE("mkapi_orchestra_client works") {
  mkapi_orchestra_client_uptr client{mkapi_orchestra_client_new()};
  mkapi_orchestra_client_set_available_bandwidth(client.get(), "131072");
  mkapi_orchestra_client_set_device_token(client.get(), "XXX-YYY-ZZZ");
  mkapi_orchestra_client_set_ca_bundle_path(client.get(), "cacert.pem");
  mkapi_orchestra_client_set_geoip_country_path(client.get(), "country.mmdb");
  mkapi_orchestra_client_set_geoip_asn_path(client.get(), "asn.mmdb");
  mkapi_orchestra_client_set_language(client.get(), "it_IT");
  mkapi_orchestra_client_set_network_type(client.get(), "wifi");
  mkapi_orchestra_client_set_platform(client.get(), "linux");
  mkapi_orchestra_client_set_probe_asn(client.get(), "AS30722");
  mkapi_orchestra_client_set_probe_cc(client.get(), "IT");
  mkapi_orchestra_client_set_probe_family(client.get(), "xxx-zzz");
  mkapi_orchestra_client_set_probe_timezone(client.get(), "Europe/Rome");
  mkapi_orchestra_client_set_registry_url(
      client.get(), "https://registry.proteus.test.ooni.io");
  mkapi_orchestra_client_set_secrets_file(client.get(), ".orchestra.json");
  mkapi_orchestra_client_set_software_name(client.get(), "dummy");
  mkapi_orchestra_client_set_software_version(client.get(), "0.1.0");
  mkapi_orchestra_client_add_supported_test(client.get(), "ndt");
  mkapi_orchestra_client_set_timeout(client.get(), 17);
  {
    mkapi_orchestra_result_uptr rv{mkapi_orchestra_client_sync(client.get())};
    REQUIRE(mkapi_orchestra_result_good(rv.get()));
    std::clog << "=== BEGIN REGISTER LOGS ===" << std::endl;
    std::clog << mkapi_orchestra_result_moveout_logs(rv);
    std::clog << "=== END REGISTER LOGS ===" << std::endl;
  }
  {
    mkapi_orchestra_client_set_network_type(client.get(), "mobile");
    mkapi_orchestra_result_uptr rv{mkapi_orchestra_client_sync(client.get())};
    REQUIRE(mkapi_orchestra_result_good(rv.get()));
    std::clog << "=== BEGIN UPDATE LOGS ===" << std::endl;
    std::clog << mkapi_orchestra_result_moveout_logs(rv);
    std::clog << "=== END UPDATE LOGS ===" << std::endl;
  }
}
