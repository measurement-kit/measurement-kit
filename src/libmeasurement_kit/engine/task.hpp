// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_ENGINE_TASK_HPP
#define SRC_LIBMEASUREMENT_KIT_ENGINE_TASK_HPP

#include <measurement_kit/internal/engine/task.hpp>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <thread>
#include <mutex>

#include "src/libmeasurement_kit/common/reactor.hpp"

#include <measurement_kit/common/shared_ptr.hpp>

namespace mk {
namespace engine {

class TaskImpl {
  public:
    std::condition_variable cond;
    std::deque<nlohmann::json> deque;
    std::atomic_bool interrupted{false};
    std::mutex mutex;
    SharedPtr<Reactor> reactor = Reactor::make();
    std::atomic_bool running{false};
    std::thread thread;
};

} // namespace engine
} // namespace mk
#endif
