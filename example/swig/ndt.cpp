// Public domain 2017, Simone Basso <bassosimone@gmail.com.

#include <measurement_kit/swig.hpp>

#include <stdlib.h>

#include <iostream>

int main() {
    std::string settings = R"({
        "type": "Ndt",
        "verbosity": "INFO"
    })";
    mk::swig::Task task;
    auto rv = task.initialize_ex(settings);
    if (!rv.result) {
        std::clog << "ERROR: " << rv.reason << std::endl;
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
