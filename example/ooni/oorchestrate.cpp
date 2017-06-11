// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

#include <iostream>

#include <unistd.h>

// XXX I believe there are portability issues with this header
#include <getopt.h>

#define USAGE "oorchestrate"

using namespace mk::ooni;
using namespace mk;

int main(int /*argc*/, char ** /*argv*/) {
    orchestrate::Client client;
    client.logger->set_verbosity(MK_LOG_DEBUG2);
    client.probe_cc = "IT";
    client.probe_asn = "AS0";
    client.platform = "macos";
    client.software_name = "oorchestrate";
    client.software_version = "1.0.0";
    client.supported_tests = {"web_connectivity"};
    client.network_type = "wifi";
    client.available_bandwidth = "10";
    //client.device_token = "X0";  /* Not needed on PC devices */
    client.registry_url = orchestrate::testing_registry_url();
    std::promise<Error> promise;
    std::future<Error> future = promise.get_future();
    client.register_probe([client, &promise](Error &&error) {
        if (error) {
            promise.set_value(error);
            return;
        }
        client.update([&promise](Error &&error) {
            promise.set_value(error);
        });
    });
    future.wait();
}
