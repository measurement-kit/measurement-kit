// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

using namespace mk::ooni;
using namespace mk;

int main(int /*argc*/, char ** /*argv*/) {
    orchestrate::Client client;
    client.logger->set_verbosity(MK_LOG_DEBUG2);
    client.geoip_country_path = "GeoIP.dat";
    client.geoip_asn_path = "GeoIPASNum.dat";
    client.network_type = "wifi";
    //client.device_token = "{TOKEN}";  /* Not needed on PC devices */
    client.registry_url = orchestrate::testing_registry_url();
    std::promise<Error> promise;
    std::future<Error> future = promise.get_future();
    client.update([&promise](Error &&error) {
        promise.set_value(error);
    });
    future.wait();
}
