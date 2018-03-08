// Public domain 2017, Simone Basso <bassosimone@gmail.com.

#include <measurement_kit/swig.hpp>

#include <stdlib.h>

#include <iostream>

int main() {
    std::string settings = R"({
        "name": "Ndt",
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
