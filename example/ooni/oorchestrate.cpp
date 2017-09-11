// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <future>
#include <measurement_kit/ooni.hpp>
#include <unistd.h>

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
    Auth auth;
    auto run_with_promise =
        [&](std::function<void(std::promise<Error> &)> &&f) {
            std::promise<Error> promise;
            std::future<Error> future = promise.get_future();
            f(promise);
            if (future.get() != NoError()) {
                client.logger->warn("Operation failed; terminating");
                exit(1);
            }
        };
    auto process_result = [&](std::promise<Error> &promise, Error &&error,
                              Auth &&new_auth) {
        if (!!error) {
            error = auth.dump(path);
        }
        std::swap(auth, new_auth);
        promise.set_value(error);
    };
    run_with_promise([&](std::promise<Error> &promise) {
        Error error = auth.load(path);
        if (!!error) {
            client.register_probe("", [&](Error &&error, Auth &&new_auth) {
                process_result(promise, std::move(error), std::move(new_auth));
            });
        } else {
            promise.set_value(NoError());
        }
    });
    auto simulate_network_change = [&](std::string network_type) {
        run_with_promise([&](std::promise<Error> &promise) {
            client.network_type = network_type;
            client.update(std::move(auth), [&](Error &&error, Auth &&new_auth) {
                process_result(promise, std::move(error), std::move(new_auth));
            });
        });
    };
    simulate_network_change("3g");
    simulate_network_change("wifi");
}
