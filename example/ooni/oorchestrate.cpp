// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

using namespace mk::ooni::orchestrate;
using namespace mk::ooni;
using namespace mk;

int main(int /*argc*/, char ** /*argv*/) {
    const std::string path = "orchestrator_secrets.json";
    Client client;
    client.logger->set_verbosity(MK_LOG_DEBUG2);
    client.geoip_country_path = "GeoIP.dat";
    client.geoip_asn_path = "GeoIPASNum.dat";
    client.network_type = "wifi";
    // client.device_token = "{TOKEN}";  /* Not needed on PC devices */
    client.registry_url = testing_registry_url();
    std::promise<Error> promise;
    std::future<Error> future = promise.get_future();
    Auth auth;
    Error err = auth.load(path);
    if (err) {
        client.register_probe("",
                              [&promise, &path](Error &&error, Auth &&auth) {
                                  if (!error) {
                                      error = auth.dump(path);
                                  }
                                  promise.set_value(error);
                              });
        err = future.get();
        return (err) ? 1 : 0;
    }
    client.update(std::move(auth), [&promise](Error &&error, Auth &&) {
        promise.set_value(error);
    });
    err = future.get();
    return (err) ? 1 : 0;
}
