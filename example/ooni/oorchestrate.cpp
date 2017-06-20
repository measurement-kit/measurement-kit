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
    auto func = mk::fcompose(
          mk::fcompose_policy_async(),
          [&err, &client, &path](Auth &&auth,
                                 Callback<Error &&, Auth &&> &&cb) {
              if (!err) {
                  // If we have loaded the authentication, proceed
                  cb(NoError(), std::move(auth));
                  return;
              }
              client.register_probe("", [&path, cb = std::move(cb) ](
                                              Error && error, Auth && auth) {
                  if (error) {
                      cb(std::move(error), {});
                      return;
                  }
                  if ((error = auth.dump(path)) != NoError()) {
                      cb(std::move(error), {});
                      return;
                  }
                  cb(NoError(), std::move(auth));
              });
          },
          [&client](Error &&error, Auth &&auth,
                    Callback<Error &&, Auth &&> &&cb) {
              if (error) {
                  cb(std::move(error), {});
                  return;
              }
              client.network_type = "3g"; // Simulate network change
              client.update(std::move(auth), std::move(cb));
          },
          [&client](Error &&error, Auth &&auth,
                    Callback<Error &&, Auth &&> &&cb) {
              // Second update to check whether the auth token is working
              if (error) {
                  cb(std::move(error), std::move(auth));
                  return;
              }
              client.network_type = "wifi"; // Simulate network change
              client.update(std::move(auth), std::move(cb));
          });
    func(std::move(auth),
         [&promise](Error &&error, Auth &&) { promise.set_value(error); });
    return (future.get()) ? 1 : 0;
}
