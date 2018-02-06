// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define MK_ENGINE_INTERNALS
#include <measurement_kit/engine.h>

#include <assert.h>
#include <stdint.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>

#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/nlohmann/json.hpp>
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/common/shared_ptr.hpp>

#include "src/libmeasurement_kit/nettests/runnable.hpp"

namespace mk {
namespace engine {

// # Multi-thread stuff
//
// Comes first because it needs more careful handling.

class Semaphore {
  public:
    Semaphore() = default;

    void wait() {
        std::unique_lock<std::mutex> lock{mutex_};
        cond_.wait(lock, [this]() { return !active_; });
        active_ = true;
    }

    void signal() {
        {
            std::unique_lock<std::mutex> _{mutex_};
            active_ = false;
        }
        cond_.notify_one();
    }

    ~Semaphore() = default;

  private:
    bool active_ = false;
    std::condition_variable cond_;
    std::mutex mutex_;
};

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

static void task_run(TaskImpl *pimpl, const nlohmann::json &settings);
static bool is_event_valid(const std::string &);

static void emit(TaskImpl *pimpl, nlohmann::json &&event) {

    // In debug mode, make sure that we're emitting an event that we know
    assert(event.is_object());
    assert(event.count("type") != 0);
    assert(event.at("type").is_string());
    assert(is_event_valid(event.at("type").get<std::string>()));

    // Actually emit the event.
    {
        std::unique_lock<std::mutex> _{pimpl->mutex};
        pimpl->deque.push_back(std::move(event));
    }
    pimpl->cond.notify_all(); // more efficient if unlocked
}

Task::Task(nlohmann::json &&settings) {
    pimpl_ = std::make_unique<TaskImpl>();
    std::promise<void> barrier;
    std::future<void> started = barrier.get_future();
    pimpl_->thread = std::thread([this, &barrier,
                                         settings = std::move(settings)]() {
        pimpl_->running = true;
        barrier.set_value();
        static Semaphore semaphore;
        semaphore.wait(); // block until a previous task has finished running
        task_run(pimpl_.get(), settings);
        pimpl_->running = false;
        pimpl_->cond.notify_one(); // tell the reader we're done
        semaphore.signal();        // allow another task to run
    });
    started.wait(); // guarantee Task() completes when the thread is running
}

void Task::interrupt() {
    // both variables are safe to use in a MT context
    pimpl_->reactor->stop();
    pimpl_->interrupted = true;
}

nlohmann::json Task::wait_for_next_event() {
    std::unique_lock<std::mutex> lock{pimpl_->mutex};
    pimpl_->cond.wait(lock,
            [this]() { return !pimpl_->running || !pimpl_->deque.empty(); });
    if (!pimpl_->deque.empty()) {
        auto rv = std::move(pimpl_->deque.front());
        pimpl_->deque.pop_front();
        return nlohmann::json{std::move(rv)};
    }
    assert(!pimpl_->running);
    return nlohmann::json(); // this is a `null` JSON object
}

Task::~Task() {
    if (pimpl_->thread.joinable()) {
        pimpl_->thread.join();
    }
}

// # Helpers

static std::tuple<int, bool> verbosity_atoi(const std::string &str) {
#define ATOI(value)                                                            \
    if (str == #value) {                                                       \
        return std::make_tuple(MK_LOG_##value, true);                          \
    }
    MK_ENUM_VERBOSITY(ATOI)
#undef ATOI
    return std::make_tuple(0, false);
}

static std::tuple<std::string, bool> verbosity_itoa(int n) {
#define ITOA(value)                                                            \
    if (n == MK_LOG_##value) {                                                 \
        return std::make_tuple(std::string{#value}, true);                     \
    }
    MK_ENUM_VERBOSITY(ITOA)
#undef ITOA
    return std::make_tuple(std::string{}, false);
}

static nlohmann::json make_log_event(uint32_t verbosity, const char *message) {
    auto verbosity_tuple = verbosity_itoa(verbosity);
    assert(std::get<1>(verbosity_tuple));
    const std::string &vs = std::get<0>(verbosity_tuple);
    return nlohmann::json{
            {"type", "LOG"}, {"verbosity", vs}, {"message", message}};
}

static nlohmann::json make_failure_event(const Error &error) {
    return nlohmann::json{{"type", "FAILURE"}, {"failure", error.reason}};
}

static bool is_event_valid(const std::string &str) {
    bool rv = false;
    do {
#define CHECK(value)                                                           \
    if (#value == str) {                                                       \
        rv = true;                                                             \
        break;                                                                 \
    }
        MK_ENUM_EVENT(CHECK)
#undef CHECK
    } while (0);
    return rv;
}

static std::unique_ptr<nettests::Runnable> make_runnable(const std::string &s) {
    std::unique_ptr<nettests::Runnable> runnable;
#define ATOP(value)                                                            \
    if (s == #value) {                                                         \
        runnable.reset(new nettests::value##Runnable);                         \
    }
    MK_ENUM_TASK(ATOP)
#undef ATOP
    return runnable;
}

static void emit_settings_failure(TaskImpl *pimpl, const char *reason) {
    emit(pimpl, make_log_event(MK_LOG_WARNING, reason));
    emit(pimpl, make_failure_event(ValueError()));
}

// # Run task

static void task_run(TaskImpl *pimpl, const nlohmann::json &settings) {

    // make sure that `settings` is an object
    if (!settings.is_object()) {
        emit_settings_failure(pimpl, "invalid settings type");
        return;
    }

    // extract and process `type`
    std::unique_ptr<nettests::Runnable> runnable;
    {
        if (settings.count("type") == 0) {
            emit_settings_failure(pimpl, "missing key: type");
            return;
        }
        if (!settings.at("type").is_string()) {
            emit_settings_failure(pimpl, "invalid key type: type");
            return;
        }
        std::string type = settings.at("type").get<std::string>();
        runnable = make_runnable(type);
    }
    if (!runnable) {
        emit_settings_failure(pimpl, "unknown task type");
        return;
    }
    runnable->reactor = pimpl->reactor; // default is nullptr, we must set it

    // extract and process `verbosity`
    {
        uint32_t verbosity = MK_LOG_QUIET;
        if (settings.count("verbosity") != 0) {
            if (!settings.at("verbosity").is_string()) {
                emit_settings_failure(pimpl, "invalid type: verbosity");
                return;
            }
            auto verbosity_string = settings.at("verbosity").get<std::string>();
            auto verbosity_tuple = verbosity_atoi(verbosity_string);
            bool okay = std::get<1>(verbosity_tuple);
            if (!okay) {
                emit_settings_failure(pimpl, "unknown verbosity level");
                return;
            }
            verbosity = std::get<0>(verbosity_tuple);
        }
        runnable->logger->set_verbosity(verbosity);
    }

    // extract 'enabled_events'
    std::set<std::string> enabled_events;
    if (settings.count("enabled_events") != 0) {
        if (!settings.at("enabled_events").is_array()) {
            emit_settings_failure(pimpl, "invalid type: enabled_events");
            return;
        }
        for (auto &entry : settings.at("enabled_events")) {
            if (!entry.is_string()) {
                emit_settings_failure(pimpl, "invalid type for event");
                return;
            }
            std::string s = entry.get<std::string>();
            if (!is_event_valid(s)) {
                emit_settings_failure(pimpl, "unknown event");
                return;
            }
            enabled_events.insert(s);
        }
    }

    // see whether 'PERFORMANCE' is enabled
    if (enabled_events.count("PERFORMANCE") != 0) {
        runnable->logger->on_event([pimpl](const char *line) {
            nlohmann::json event;
            try {
                auto inner = nlohmann::json::parse(line);
                if (inner.at("type") == "download-speed") {
                    event["direction"] = "download";
                } else if (inner.at("type") == "upload-speed") {
                    event["direction"] = "upload";
                } else {
                    assert(false);
                    return; // Not an event we wanted to filter
                }
                event["type"] = "PERFORMANCE";
                event["elapsed_seconds"] = inner["elapsed"][0];
                event["num_streams"] = inner["num_streams"];
                event["speed_kbit_s"] = inner["speed"][0];
            } catch (const std::exception &) {
                assert(false);
                return; // Perhaps not the right event format
            }
            emit(pimpl, std::move(event));
        });
    }

    // see whether 'LOG' is enabled
    if (enabled_events.count("LOG") != 0) {
        runnable->logger->on_log([pimpl](uint32_t verbosity, const char *line) {
            if ((verbosity & ~MK_LOG_VERBOSITY_MASK) != 0) {
                return; // mask out non-logging events
            }
            emit(pimpl, make_log_event(verbosity, line));
        });
    }

    // start the task (reactor and interrupted are MT safe)
    pimpl->reactor->run_with_initial_event([&]() {
        if (pimpl->interrupted) {
            return; // allow for early interruption
        }
        runnable->begin([&](Error) {
            runnable->end([&](Error) {
                // NOTHING
            });
        });
    });
}

} // namespace engine
} // namespace mk
