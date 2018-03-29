// Public domain 2017, Simone Basso <bassosimone@gmail.com.

#define MK_ENGINE_INTERNALS
#include <measurement_kit/engine.h>

#include <iostream>

#include <measurement_kit/common/nlohmann/json.hpp>

int main() {
    nlohmann::json settings{
        {"name", "Ndt"},
        {"log_level", "INFO"},
    };
    std::clog << settings.dump() << "\n";
    mk::engine::Task task{std::move(settings)};
    while (!task.is_done()) {
        auto event = task.wait_for_next_event();
        std::clog << event << "\n";
    }
}
