// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/fcompose.hpp"
#include <measurement_kit/ooni.hpp>

#include <unistd.h>

#include <future>

using namespace mk::ooni::orchestrate;
using namespace mk::ooni;
using namespace mk;

#define USAGE "oorchestrate [-v]"

int main(int argc, char **argv) {
    Client client;
    for (int ch; (ch = getopt(argc, argv, "v")) != -1;) {
        switch (ch) {
        case 'v':
            client.logger->increase_verbosity();
            break;
        default:
            std::cout << USAGE << "\n";
            exit(1);
        }
    }
    argc -= optind, argv += optind;
    if (argc > 0) {
        std::cout << USAGE << "\n";
        exit(1);
    }
    const std::string path = "orchestrator_secrets.json";
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
          [&client, &path](Error &&error, Auth &&auth,
                           Callback<Error &&, Auth &&> &&cb) {
              // Second update to check whether the auth token is working
              if (error) {
                  cb(std::move(error), std::move(auth));
                  return;
              }
              // Dump after update() so we have also the token stored on disk
              if ((error = auth.dump(path)) != NoError()) {
                  cb(std::move(error), {});
                  return;
              }
              client.network_type = "wifi"; // Simulate network change
              client.update(std::move(auth), std::move(cb));
          });
    func(std::move(auth),
         [&promise](Error &&error, Auth &&) { promise.set_value(error); });
    return (future.get()) ? 1 : 0;
}
