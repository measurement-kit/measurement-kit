// Public domain 2017, Simone Basso <bassosimone@gmail.com.

#include <measurement_kit/swig.hpp>

#include <stdlib.h>

#include <iostream>

#include <measurement_kit/common/nlohmann/json.hpp>

int main() {
    std::string settings = R"({
        "enabled_events": ["LOG", "PERFORMANCE"],
        "type": "Ndt",
        "verbosity": "INFO"
    })";
    mk::swig::Task task;
    bool okay = task.initialize(settings);
    if (!okay) {
        std::clog << "cannot parse json" << std::endl;
        exit(1);
    }
    while (true) {
        std::string event = task.wait_for_next_event();
        if (event == "null") {
            break; // we have got the `null` JSON object, we're done
        }
        std::clog << event << "\n";
    }
}
