// Public domain 2017, Simone Basso <bassosimone@gmail.com.

#define MK_EXPOSE_SWIG_API
#include <measurement_kit/swig.hpp>

#include <stdlib.h>

#include <iostream>

int main() {
    std::string settings = R"({
        "inputs": [
            "https://slashdot.org/",
            "http://www.microsoft.com"
        ],
        "name": "WebConnectivity",
        "log_level": "INFO"
    })";
    mk::swig::Task task;
    auto rv = task.initialize_ex(settings);
    if (!rv.result) {
        std::clog << "ERROR: " << rv.reason << std::endl;
        exit(1);
    }
    while (!task.is_done()) {
        std::string event = task.wait_for_next_event();
        std::clog << event << "\n";
    }
}