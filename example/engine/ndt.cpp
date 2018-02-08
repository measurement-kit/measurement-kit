// Public domain 2017, Simone Basso <bassosimone@gmail.com.

#define MK_ENGINE_INTERNALS
#include <measurement_kit/engine.h>

#include <iostream>

#include <measurement_kit/common/nlohmann/json.hpp>

int main() {
    nlohmann::json settings{
        {"type", "Ndt"},
        {"verbosity", "INFO"},
    };
    std::clog << settings.dump() << "\n";
    mk::engine::Task task{std::move(settings)};
    while (true) {
        auto event = task.wait_for_next_event();
        if (event == nullptr) {
            break;
        }
        std::clog << event << "\n";
    }
}
