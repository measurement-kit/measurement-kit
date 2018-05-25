// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_FFI_HPP
#define MEASUREMENT_KIT_NETTESTS_FFI_HPP

// Inline reimplementation of Measurement Kit's original API in terms
// of the new <measurement_kit/ffi.h> API.

#include <stdint.h>

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include <measurement_kit/common/data_usage.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/nlohmann/json.hpp>
#include <measurement_kit/common/shared_ptr.hpp>

#include <measurement_kit/ffi.h>
#include <measurement_kit/ffi_macros.h>

#if __cplusplus >= 201402L && !defined MK_NETTESTS_INTERNAL
#define MK_NETTESTS_DEPRECATED [[deprecated]]
#else
#define MK_NETTESTS_DEPRECATED /* Nothing */
#endif

namespace mk {
namespace nettests {

class TaskDeleter {
  public:
    void operator()(mk_task_t *task) noexcept { mk_task_destroy(task); }
};
using TaskUptr = std::unique_ptr<mk_task_t, TaskDeleter>;

class EventDeleter {
  public:
    void operator()(mk_event_t *event) noexcept { mk_event_destroy(event); }
};
using EventUptr = std::unique_ptr<mk_event_t, EventDeleter>;

class MK_NETTESTS_DEPRECATED BaseTest {
  public:
    // Implementation notes
    // --------------------
    //
    // 1. the code in here should work with both nlohmann/json v2 and v3 as
    // long as we do not catch any exception. In fact, v2 used to throw standard
    // exceptions (i.e. `std::exception`) while v3 has its own exceptions;
    //
    // 2. as a result, we're going to assume that the JSON objects passed to us
    // by the FFI API of MK are always containing the fields we expect;
    //
    // 3. compared to the original implementation, this API allows to have
    // multiple callbacks registered for any kind of event. In the original
    // code, only _some_ events accepted multiple callbacks.

    class Details {
      public:
        nlohmann::json settings;
        std::vector<std::function<void()>> logger_eof_cbs;
        std::vector<std::function<void(uint32_t, const char *)>> log_cbs;
        std::vector<std::function<void(const char *)>> event_cbs;
        std::vector<std::function<void(double, const char *)>> progress_cbs;
        uint32_t log_level = MK_LOG_WARNING;
        std::vector<std::function<void(std::string)>> entry_cbs;
        std::vector<std::function<void()>> begin_cbs;
        std::vector<std::function<void()>> end_cbs;
        std::vector<std::function<void()>> destroy_cbs;
        std::vector<std::function<void(DataUsage)>> overall_data_usage_cbs;
    };

    BaseTest() { impl_.reset(new Details); }

    // The original implementation had a virtual destructor but no other
    // virtual members. Hence in the reimplementation I am removing the
    // attribute `virtual` since it seems unnecessary.
    ~BaseTest() {}

    // Setters
    // -------
    //
    // All the following methods are straightforward because they just
    // configure the settings or register specific callbacks.

    BaseTest &add_input(std::string s) {
        impl_->settings["inputs"].push_back(std::move(s));
        return *this;
    }

    BaseTest &set_input_filepath(std::string s) {
        impl_->settings["input_filepaths"].clear();
        return add_input_filepath(std::move(s));
    }

    BaseTest &add_input_filepath(std::string s) {
        impl_->settings["input_filepaths"].push_back(std::move(s));
        return *this;
    }

    BaseTest &set_output_filepath(std::string s) {
        impl_->settings["output_filepath"] = std::move(s);
        return *this;
    }

    BaseTest &set_error_filepath(std::string s) {
        impl_->settings["log_filepath"] = std::move(s);
        return *this;
    }

    BaseTest &on_logger_eof(std::function<void()> &&fn) {
        impl_->logger_eof_cbs.push_back(std::move(fn));
        return *this;
    }

    BaseTest &on_log(std::function<void(uint32_t, const char *)> &&fn) {
        impl_->log_cbs.push_back(std::move(fn));
        return *this;
    }

    BaseTest &on_event(std::function<void(const char *)> &&fn) {
        impl_->event_cbs.push_back(std::move(fn));
        return *this;
    }

    BaseTest &on_progress(std::function<void(double, const char *)> &&fn) {
        impl_->progress_cbs.push_back(std::move(fn));
        return *this;
    }

    BaseTest &set_verbosity(uint32_t level) {
        impl_->log_level = level;
        return *this;
    }

    BaseTest &increase_verbosity() {
        if (impl_->log_level < MK_LOG_DEBUG2) {
            ++impl_->log_level;
        }
        return *this;
    }

    template <typename T, typename = typename std::enable_if<
                                  std::is_arithmetic<T>::value>::type>
    BaseTest &set_option(std::string key, T value) {
        impl_->settings["options"][key] = value;
        return *this;
    }

    BaseTest &set_option(std::string key, std::string value) {
        impl_->settings["options"][key] = value;
        return *this;
    }

    BaseTest &add_annotation(std::string key, std::string value) {
        impl_->settings["annotations"][key] = value;
        return *this;
    }

    BaseTest &on_entry(std::function<void(std::string)> &&fn) {
        impl_->entry_cbs.push_back(std::move(fn));
        return *this;
    }

    BaseTest &on_begin(std::function<void()> &&fn) {
        impl_->begin_cbs.push_back(std::move(fn));
        return *this;
    }

    BaseTest &on_end(std::function<void()> &&fn) {
        impl_->end_cbs.push_back(std::move(fn));
        return *this;
    }

    BaseTest &on_destroy(std::function<void()> &&fn) {
        impl_->destroy_cbs.push_back(std::move(fn));
        return *this;
    }

    BaseTest &on_overall_data_usage(std::function<void(DataUsage)> &&fn) {
        impl_->overall_data_usage_cbs.push_back(std::move(fn));
        return *this;
    }

    // run() & start()
    // ---------------
    //
    // We should not be able to run more than a test with this class
    // hence we immediately swap the context. Because we're using a
    // SharedPtr, this means that attempting to run more than one test
    // leads to an exception being thrown.

    void run() { run_static(std::move(impl_)); }

    void start(std::function<void()> &&fn) {
        std::thread thread{[tip = std::move(impl_), fn = std::move(fn)]() {
            run_static(std::move(tip));
            fn();
        }};
        thread.detach();
    }

  private:
    // Task running
    // ------------
    //
    // How we actually start a task and process its events.

    // Helper macro used to facilitate suppressing exceptions since the
    // nettest.hpp API always suppresses exceptions in callbacks. This is
    // consistent with the original implementation's behavior.
    //
    // This is not necessarily a very good idea, but the original code was
    // doing that, hence we should do that here as well.
#define MK_NETTESTS_CALL_AND_SUPPRESS(func, args)                              \
    do {                                                                       \
        try {                                                                  \
            func args;                                                         \
        } catch (...) {                                                        \
            /* SUPPRESS */                                                     \
        }                                                                      \
    } while (0)

    static void run_static(SharedPtr<Details> tip) {
        switch (tip->log_level) {
        case MK_LOG_ERR:
            tip->settings["log_level"] = "ERR";
            break;
        case MK_LOG_WARNING:
            tip->settings["log_level"] = "WARNING";
            break;
        case MK_LOG_INFO:
            tip->settings["log_level"] = "INFO";
            break;
        case MK_LOG_DEBUG:
            tip->settings["log_level"] = "DEBUG";
            break;
        case MK_LOG_DEBUG2:
            tip->settings["log_level"] = "DEBUG2";
            break;
        default:
            assert(false); // Should not happen
            break;
        }
        // Serializing the settings MAY throw if we provided strings
        // that are non-JSON serializeable. For now, let the exception
        // propagate if that unexpected condition occurs.
        //
        // TODO(bassosimone): since this error condition did not happen
        // with the previous iteration of this API, it's an open question
        // whether to handle this possible error condition or not.
        std::string serialized_settings;
        serialized_settings = tip->settings.dump();
        TaskUptr tup{mk_task_start(serialized_settings.c_str())};
        if (!tup) {
            throw std::runtime_error("mk_task_start() failed");
        }
        while (!mk_task_is_done(tup.get())) {
            nlohmann::json ev;
            {
                EventUptr eup{mk_task_wait_for_next_event(tup.get())};
                if (!eup) {
                    throw std::runtime_error(
                            "mk_task_wait_for_next_event() failed");
                }
                const char *s = mk_event_serialize(eup.get());
                assert(s != nullptr);
#ifdef MK_NETTESTS_TRACE_EVENTS
                std::clog << "mk::nettests: got event: " << s << std::endl;
#endif
                // The following statement MAY throw. Since we do not expect
                // MK to serialize a non-parseable JSON, just let the eventual
                // exception propagate and terminate the program.
                ev = nlohmann::json::parse(s);
            }
            process_event(tip, std::move(ev));
        }
        for (auto &cb : tip->logger_eof_cbs) {
            MK_NETTESTS_CALL_AND_SUPPRESS(cb, ());
        }
        for (auto &cb : tip->destroy_cbs) {
            MK_NETTESTS_CALL_AND_SUPPRESS(cb, ());
        }
    }

    // Events processing
    // -----------------
    //
    // Map events emitted by the FFI API to nettests.hpp callbacks. This is the
    // section where most of the complexity is.

    static void process_event(
            const SharedPtr<Details> &tip, nlohmann::json ev) {
        // Implementation notes:
        //
        // 1) as mentioned above, in processing events we're quite strict in the
        // sense that we _assume_ events to have a specific structure and fail
        // with an unhandled exception otherwise;
        //
        // 2) the nettests API is less rich that the FFI API; as such, there
        // are several FFI events that are going to be ignored.
        std::string key = ev.at("key");
        // TODO(bassosimone): make sure events names are OK.
        if (key == "failure.measurement") {
            // NOTHING
        } else if (key == "failure.measurement_submission") {
            // NOTHING
        } else if (key == "failure.startup") {
            // NOTHING
        } else if (key == "log") {
            std::string log_level = ev.at("value").at("log_level");
            std::string message = ev.at("value").at("message");
            uint32_t verbosity = MK_LOG_QUIET;
            if (log_level == "ERR") {
                verbosity = MK_LOG_ERR;
            } else if (log_level == "WARNING") {
                verbosity = MK_LOG_WARNING;
            } else if (log_level == "INFO") {
                verbosity = MK_LOG_INFO;
            } else if (log_level == "DEBUG") {
                verbosity = MK_LOG_DEBUG;
            } else if (log_level == "DEBUG2") {
                verbosity = MK_LOG_DEBUG2;
            } else {
                assert(false);
                return;
            }
            for (auto &cb : tip->log_cbs) {
                MK_NETTESTS_CALL_AND_SUPPRESS(cb, (verbosity, message.c_str()));
            }
        } else if (key == "measurement") {
            std::string json_str = ev.at("value").at("json_str");
            for (auto &cb : tip->entry_cbs) {
                MK_NETTESTS_CALL_AND_SUPPRESS(cb, (json_str));
            }
        } else if (key == "status.end") {
            double downloaded_kb = ev.at("value").at("downloaded_kb");
            double uploaded_kb = ev.at("value").at("uploaded_kb");
            DataUsage du;
            // There are cases where the following could overflow but, again, we
            // do not want to break the existing API.
            du.down = (uint64_t)(downloaded_kb * 1000.0);
            du.up = (uint64_t)(uploaded_kb * 1000.0);
            for (auto &cb : tip->overall_data_usage_cbs) {
                MK_NETTESTS_CALL_AND_SUPPRESS(cb, (du));
            }
            for (auto &cb : tip->end_cbs) {
                MK_NETTESTS_CALL_AND_SUPPRESS(cb, ());
            }
        } else if (key == "status.geoip_lookup") {
            // NOTHING
        } else if (key == "status.progress") {
            double percentage = ev.at("value").at("percentage");
            std::string message = ev.at("value").at("message");
            for (auto &cb : tip->progress_cbs) {
                MK_NETTESTS_CALL_AND_SUPPRESS(
                        cb, (percentage, message.c_str()));
            }
        } else if (key == "status.queued") {
            // NOTHING
        } else if (key == "status.measurement_started") {
            // NOTHING
        } else if (key == "status.measurement_uploaded") {
            // NOTHING
        } else if (key == "status.measurement_done") {
            // NOTHING
        } else if (key == "status.report_created") {
            // NOTHING
        } else if (key == "status.started") {
            for (auto &cb : tip->begin_cbs) {
                MK_NETTESTS_CALL_AND_SUPPRESS(cb, ());
            }
        } else if (key == "status.update.performance") {
            std::string direction = ev.at("value").at("direction");
            double elapsed = ev.at("value").at("elapsed");
            int64_t num_streams = ev.at("value").at("num_streams");
            double speed_kbps = ev.at("value").at("speed_kbps");
            nlohmann::json doc;
            doc["type"] = direction + "-speed";
            doc["elapsed"] = {elapsed, "s"};
            doc["num_streams"] = num_streams;
            doc["speed"] = {speed_kbps, "kbit/s"};
            // Serializing may throw but we expect MK to pass us a good
            // JSON so don't consider this possible error condition.
            const char *s = doc.dump().c_str();
            for (auto &cb : tip->event_cbs) {
                MK_NETTESTS_CALL_AND_SUPPRESS(cb, (s));
            }
        } else if (key == "status.update.websites") {
            // NOTHING
        } else if (key == "task_terminated") {
            // NOTHING
        } else {
#ifdef MK_NETTESTS_TRACE_EVENTS
            std::clog << "WARNING: mk::nettests: unhandled event: " << key
                      << std::endl;
#endif
        }
    }

#undef MK_NETTESTS_CALL_AND_SUPPRESS // Tidy up

  protected:
    // Implementation note: using a SharedPtr<T> because it's easy to
    // move around (especially into lambdas) and because it provides the
    // guarantee of throwing on null, which was a trait of the previous
    // implementation of the nettests API.
    //
    // Also: the pointer was public in the previous implementation but
    // it was also opaque, so not very useful. For this reason, it's
    // now protected in this implementation.
    SharedPtr<Details> impl_;
};

#define MK_DECLARE_TEST(_name_)                                                \
    class _name_##Test : public BaseTest {                                     \
      public:                                                                  \
        _name_##Test() : BaseTest() { impl_->settings["name"] = #_name_; }     \
    };
MK_ENUM_TASKS(MK_DECLARE_TEST)
#undef MK_DECLARE_TEST

} // namespace nettests
} // namespace mk
#endif
