// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

#include <iostream>

#include <unistd.h>

// XXX I believe there are portability issues with this header
#include <getopt.h>

#define USAGE "oorchestrate [-v] [--events-url events_url] [--registry-url registry_url] --username username --password password"

using namespace mk::ooni;
using namespace mk;

int main(int argc, char **argv) {
    Var<Logger> logger = Logger::make();
    logger->set_verbosity(MK_LOG_DEBUG2);
    orchestrate::Client client;
    client.probe_cc = "IT";
    client.probe_asn = "AS0";
    client.platform = "macos";
    client.software_name = "example";
    client.software_version = "1.0.0";
    client.registry_url = orchestrate::testing_registry_url();
    client.register_probe({}, logger,
                          [client, logger](Error &&error) mutable /* XXX */ {
                              if (error) {
                                  throw error;
                              }
                              client.update({}, logger, [](Error &&error) {
                                  if (error) {
                                      throw error;
                                  }
                              });
                          });
    for (;;) {
        sleep(1);
        if (!AsyncRunner::global()->running()) {
            break;
        }
    }

#if 0
    std::string events_url = orchestrator::testing_events_url();
    std::string registry_url = orchestrator::testing_registry_url();

    Var<orchestrator::Authentication> auth(new orchestrator::Authentication);
    Var<orchestrator::ProbeData> pd(new orchestrator::ProbeData);

    int c;
    while (1) {
      static struct option long_options[] = {
        {"events-url", required_argument, 0, 0},
        {"registry-url", required_argument, 0, 1},
        {"username", required_argument, 0, 'u'},
        {"password", required_argument, 0, 'p'},
        {"verbose", required_argument, 0, 'v'},
        {0, 0, 0, 0}
      };

      int opt_idx = 0;
      c = getopt_long(argc, argv, "up:v", long_options, &opt_idx);
      if (c == -1) {
        break;
      }

      switch (c) {
        case 0:
          events_url = optarg;
          break;
        case 1:
          registry_url = optarg;
          break;
        case 'u':
          auth->username = optarg;
          break;
        case 'p':
          auth->password = optarg;
          break;
        case 'v':
          increase_verbosity();
          break;

        default:
          std::cout << USAGE << "\n";
          exit(1);
      }
    }
    auth->base_url = registry_url;

    argc -= optind;
    argv += optind;

    debug("oorchestrate: starting");
    loop_with_initial_event([&]() {
        pd->probe_cc = "IT";
        pd->probe_asn = "AS0";
        pd->platform = "macos";
        pd->software_name = "example";
        pd->software_version = "1.0.0";
        debug("oorchestrate: registering probe");
        pd->register_probe(registry_url, auth->password,
            [=](Error error, std::string client_id){
          if (error) {
            std::cout << "Failed to register: " << error.code;
            break_loop();
            return;
          }
          auth->username = client_id;
          info("registered with ID: %s",
               client_id.c_str());
          pd->token = "DUMMY-TOKEN";
          debug("oorchestrate: updating probe data");
          pd->update(registry_url,
                     auth, [=](Error error){
            debug("oorchestrate: updated probe data");
            if (error) {
              std::cout << "Failed to update: " << error.code;
              break_loop();
              return;

            }
            debug("oorchestrate: listing tasks");
            list_tasks(events_url, auth,
                [=](Error error, std::vector<orchestrator::Task> task_list){
                debug("oorchestrate: listed tasks");
                if (error) {
                  std::cout << "Failed to list tasks: " << error.code;
                  break_loop();
                  return;

                }
                for (auto task : task_list) {
                  info("Found task: %s", task.task_id.c_str());
                }
                break_loop();
            });
          });
        });
    });

    return 0;
#endif
}
