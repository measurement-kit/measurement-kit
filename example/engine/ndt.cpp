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
    // TODO(bassosimone): replace is_running() with is_done(), as discussed
    // with @hellais, so that we capture the moment where the task has
    // stopped _and_ the queue is drained. With the current impl in fact
    // we are losing the final events.
    while (task.is_running()) {
        auto event = task.wait_for_next_event();
        std::clog << event << "\n";
    }
}
