// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define MK_ENGINE_INTERNALS // we need to access engine internals

#include <measurement_kit/ffi.h>

#include <exception>
#include <new>
#include <string>

#include <measurement_kit/common/nlohmann/json.hpp>
#include <measurement_kit/engine.h>

struct mk_event_ : public std::string {
    using std::string::string;
};

static mk_event_t *mk_event_create(const nlohmann::json &json) noexcept {
    return new (std::nothrow) mk_event_t{json.dump().data()};
}

const char *mk_event_serialize(mk_event_t *event) noexcept {
    return (event) ? event->data() : nullptr;
}

void mk_event_destroy(mk_event_t *event) noexcept {
    delete event; // handles nullptr
}

struct mk_task_ : mk::engine::Task {
    using mk::engine::Task::Task;
};

mk_task_t *mk_task_start(const char *settings) noexcept {
    if (settings == nullptr) {
        return nullptr;
    }
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(settings);
    } catch (const std::exception &) {
        return nullptr;
    }
    return new (std::nothrow) mk_task_t{std::move(json)};
}

mk_event_t *mk_task_wait_for_next_event(mk_task_t *task) noexcept {
    return (task) ? mk_event_create(task->wait_for_next_event()) : nullptr;
}

void mk_task_interrupt(mk_task_t *task) noexcept {
    if (task != nullptr) {
        task->interrupt();
    }
}

void mk_task_destroy(mk_task_t *task) noexcept {
    delete task; // handles nullptr
}
