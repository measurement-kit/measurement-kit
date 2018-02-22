// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define MK_ENGINE_INTERNALS // we need to access engine internals

#include <measurement_kit/ffi.h>

#include <limits.h>
#include <string.h>

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
    return mk_task_start_ex(settings, nullptr, -1);
}

static void safe_copy(const char *reason, char *errbuf, size_t errbuf_siz) {
    if (errbuf != nullptr && errbuf_siz > 0) {
        size_t size = strlen(reason);
        if (size > SIZE_MAX - 1) {
            return;
        }
        size += 1; // account for the final '\0'
        if (size < errbuf_siz) {
            memcpy(errbuf, reason, size);
            return;
        }
        // deal with truncation, possibly hinting about that
        memcpy(errbuf, reason, errbuf_siz);
        errbuf[errbuf_siz - 1] = '\0'; // zero terminate
        if (errbuf_siz > 6) {
            errbuf[errbuf_siz - 2] = ']';
            errbuf[errbuf_siz - 3] = '.';
            errbuf[errbuf_siz - 4] = '.';
            errbuf[errbuf_siz - 5] = '.';
            errbuf[errbuf_siz - 6] = '[';
            errbuf[errbuf_siz - 7] = ' ';
        }
    }
}

mk_task_t *mk_task_start_ex(
        const char *settings, char *errbuf, size_t errbuf_siz) noexcept {
    if (errbuf != nullptr && errbuf_siz > 0) {
        errbuf[0] = '\0'; // initialize in any case
    }
    if (settings == nullptr) {
        safe_copy("passed null pointer", errbuf, errbuf_siz);
        return nullptr;
    }
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(settings);
    } catch (const std::exception &exc) {
        safe_copy(exc.what(), errbuf, errbuf_siz);
        return nullptr;
    }
    auto rv = new (std::nothrow) mk_task_t{std::move(json)};
    if (rv == nullptr) {
        safe_copy("out of memory", errbuf, errbuf_siz);
    }
    return rv;
}

mk_event_t *mk_task_wait_for_next_event(mk_task_t *task) noexcept {
    return (task) ? mk_event_create(task->wait_for_next_event()) : nullptr;
}

int mk_task_is_running(mk_task_t *task) noexcept {
    return (task) ? task->is_running() : 0;
}

void mk_task_interrupt(mk_task_t *task) noexcept {
    if (task != nullptr) {
        task->interrupt();
    }
}

void mk_task_destroy(mk_task_t *task) noexcept {
    delete task; // handles nullptr
}
