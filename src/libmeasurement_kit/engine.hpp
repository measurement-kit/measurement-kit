// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_ENGINE_HPP
#define SRC_LIBMEASUREMENT_KIT_ENGINE_HPP

#include <memory>

#include <measurement_kit/common/nlohmann/json.hpp>

namespace mk {
namespace engine {

class TaskImpl;

class Task {
  public:
    explicit Task(nlohmann::json &&settings);
    nlohmann::json wait_for_next_event();
    bool is_done() const;
    void interrupt();
    ~Task();

    // Implementation note: this class is _explicitly_ non copyable and non
    // movable so we don't have to worry about pimpl's validity.
    Task(const Task &) noexcept = delete;
    Task &operator=(const Task &) noexcept = delete;
    Task(Task &&) noexcept = delete;
    Task &operator=(Task &&) noexcept = delete;

  private:
    std::unique_ptr<TaskImpl> pimpl_;
};

} // namespace engine
} // namespace mk
#endif
