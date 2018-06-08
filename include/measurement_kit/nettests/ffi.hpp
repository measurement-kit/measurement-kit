// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_FFI_HPP
#define MEASUREMENT_KIT_NETTESTS_FFI_HPP

// Inline reimplementation of Measurement Kit's original API in terms
// of the new <measurement_kit/ffi.h> API.
//
// The most striking, major difference between this implementation and the
// previous implementation is the following. In the previous implementation
// tests were executed in FIFO order. In the new implementation, instead,
// they are still queued but the order of execution is random. This is fine
// since apps should actively discourage people from running parallel tests,
// using proper UX, as that is bad. The rough queuing mechanism that we
// have here is just the last line of defence against that behavior.
//
// XXX: make sure comment here is still up to date

// XXX: need to run `iwyu` again
//
// XXX: the `started` event should actually be `start`: this is a good
// moment to make sure we can cope easily with events
//
// XXX: invent a Leonidism to protect us against changes in spec

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

#include <measurement_kit/nettests/macros.h>

#include <measurement_kit/cxx14.hpp>

// Do not issue deprecation warning for us until we update to new API
#if __cplusplus >= 201402L && !defined MK_NETTESTS_INTERNAL
#define MK_NETTESTS_DEPRECATED [[deprecated]]
#else
#define MK_NETTESTS_DEPRECATED /* Nothing */
#endif

namespace mk {
namespace nettests {

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
    //
    // XXX make sure comments still make sense

    // Helper macro used to facilitate suppressing exceptions since the
    // nettest.hpp API always suppresses exceptions in callbacks. This is
    // consistent with the original implementation's behavior.
    //
    // This is not necessarily a very good idea, but the original code was
    // doing that, hence we should do that here as well.
    //
    // XXX: move down?
#define MK_NETTESTS_CALL_AND_SUPPRESS(func, args)                              \
    do {                                                                       \
        try {                                                                  \
            func args;                                                         \
        } catch (...) {                                                        \
            /* SUPPRESS */                                                     \
        }                                                                      \
    } while (0)

    BaseTest() { impl_.log_level = "WARNING"; }

    // The original implementation had a virtual destructor but no other
    // virtual members. Hence in the reimplementation I am removing the
    // attribute `virtual` since it seems unnecessary.
    ~BaseTest() {}

    // Setters
    // -------
    //
    // All the following methods are straightforward because they just
    // configure the settings or register specific callbacks.
    //
    // XXX Make sure comments still make sense

    BaseTest &add_input(std::string s) {
        impl_.add_input(s);
        return *this;
    }

    BaseTest &set_input_filepath(std::string s) {
        impl_.input_filepaths.clear();
        impl_.add_input_filepath(s);
        return *this;
    }

    BaseTest &add_input_filepath(std::string s) {
        impl_.add_input_filepath(s);
        return *this;
    }

    BaseTest &set_output_filepath(std::string s) {
        impl_.set_output_filepath(s);
        return *this;
    }

    BaseTest &set_error_filepath(std::string s) {
        impl_.set_log_filepath(s);
        return *this;
    }

    BaseTest &on_logger_eof(std::function<void()> &&fn) {
        // Note: as documented `status.end` is always emmitted just once at
        // the end of the task. As such it's perfect for clearing up resources,
        // which is the reason why `on_logger_eof()` was introduced.
        impl_.on_status_end([fn = std::move(fn)](
                const cxx14::StatusEnd &) noexcept {
            MK_NETTESTS_CALL_AND_SUPPRESS(fn, ());
        });
        return *this;
    }

    BaseTest &on_log(std::function<void(uint32_t, const char *)> &&fn) {
        impl_.on_log([fn = std::move(fn)](const cxx14::Log &info) noexcept {
            uint32_t severity = 0;
            if (info.log_level == "ERR") {
                severity = MK_LOG_ERR;
            } else if (info.log_level == "WARNING") {
                severity = MK_LOG_WARNING;
            } else if (info.log_level == "INFO") {
                severity = MK_LOG_INFO;
            } else if (info.log_level == "DEBUG") {
                severity = MK_LOG_DEBUG;
            } else if (info.log_level == "DEBUG2") {
                severity = MK_LOG_DEBUG2;
            } else {
                assert(false); // should not happen
                return;
            }
            MK_NETTESTS_CALL_AND_SUPPRESS(fn, (severity, info.message.c_str()));
        });
        return *this;
    }

    BaseTest &on_event(std::function<void(const char *)> &&fn) {
        impl_.on_status_update_performance([fn = std::move(fn)](
              const cxx14::StatusUpdatePerformance &info) noexcept {
            nlohmann::json doc;
            doc["type"] = info.direction + "-speed";
            doc["elapsed"] = {info.elapsed, "s"};
            doc["num_streams"] = info.num_streams;
            doc["speed"] = {info.speed_kbps, "kbit/s"};
            // Serializing may throw but we expect MK to pass us a good
            // JSON so don't consider this possible error condition.
            const char *s = doc.dump().c_str();
            MK_NETTESTS_CALL_AND_SUPPRESS(fn, (s));
        });
        return *this;
    }

    BaseTest &on_progress(std::function<void(double, const char *)> &&fn) {
        impl_.on_status_progress([fn = std::move(fn)](
              const cxx14::StatusProgress &info) noexcept {
            MK_NETTESTS_CALL_AND_SUPPRESS(fn, //
                  (info.percentage, info.message.c_str()));
        });
        return *this;
    }

    BaseTest &set_verbosity(uint32_t level) {
        std::string log_level;
        switch (level) {
        case MK_LOG_ERR:
            log_level = "ERR";
            break;
        case MK_LOG_WARNING:
            log_level = "WARNING";
            break;
        case MK_LOG_INFO:
            log_level = "INFO";
            break;
        case MK_LOG_DEBUG:
            log_level = "DEBUG";
            break;
        case MK_LOG_DEBUG2:
            log_level = "DEBUG2";
            break;
        default:
            assert(false); // Programmer error
            return *this;
        }
        impl_.set_log_level(log_level);
        return *this;
    }

    BaseTest &increase_verbosity() {
        if (impl_.log_level == "ERR") {
            impl_.log_level = "WARNING";
        } else if (impl_.log_level == "WARNING") {
            impl_.log_level = "INFO";
        } else if (impl_.log_level == "INFO") {
            impl_.log_level = "DEBUG";
        } else if (impl_.log_level == "DEBUG") {
            impl_.log_level = "DEBUG2";
        } else if (impl_.log_level == "DEBUG2") {
            return *this;
        } else {
            assert(false); // Internal error
            return *this;
        }
        return *this;
    }

    template <typename T, typename = typename std::enable_if<
                                  std::is_arithmetic<T>::value>::type>
    BaseTest &set_option(std::string key, T value) {
        impl_.set_option(key, value);
        return *this;
    }

    BaseTest &set_option(std::string key, std::string value) {
        impl_.set_option(key, value);
        return *this;
    }

    BaseTest &add_annotation(std::string key, std::string value) {
        impl_.set_option(key, value);
        return *this;
    }

    BaseTest &on_entry(std::function<void(std::string)> &&fn) {
        impl_.on_measurement([fn = std::move(fn)](
              const cxx14::Measurement &info) noexcept {
            MK_NETTESTS_CALL_AND_SUPPRESS(fn, (info.json_str));
        });
        return *this;
    }

    BaseTest &on_begin(std::function<void()> &&fn) {
        impl_.on_status_started([fn = std::move(fn)](
              const cxx14::StatusStarted &) noexcept {
            MK_NETTESTS_CALL_AND_SUPPRESS(fn, ());
        });
        return *this;
    }

    BaseTest &on_end(std::function<void()> &&fn) {
        impl_.on_status_end([fn = std::move(fn)](
              const cxx14::StatusEnd &) noexcept {
            MK_NETTESTS_CALL_AND_SUPPRESS(fn, ());
        });
        return *this;
    }

    BaseTest &on_destroy(std::function<void()> &&fn) {
        // As mentioned above, `on_status_end` is the right callback to map
        // onto callbacks used to clear resources when the task is done.
        impl_.on_status_end([fn = std::move(fn)](
                const cxx14::StatusEnd &) noexcept {
            MK_NETTESTS_CALL_AND_SUPPRESS(fn, ());
        });
        return *this;
    }

    BaseTest &on_overall_data_usage(std::function<void(DataUsage)> &&fn) {
        impl_.on_status_end([fn = std::move(fn)](
                const cxx14::StatusEnd &info) noexcept {
            DataUsage du;
            // There are cases where the following could overflow but, again, we
            // do not want to break the existing API.
            du.down = (uint64_t)(info.downloaded_kb * 1000.0);
            du.up = (uint64_t)(info.uploaded_kb * 1000.0);
            MK_NETTESTS_CALL_AND_SUPPRESS(fn, (du));
        });
        return *this;
    }

    // run() & start()
    // ---------------
    //
    // We should not be able to run more than a test with this class
    // hence we immediately swap the context. Because we're using a
    // SharedPtr, this means that attempting to run more than one test
    // leads to an exception being thrown.

    void run() {
        (void)cxx14::TaskRunner::global().run(std::move(impl_));
    }

    void start(std::function<void()> &&fn) {
        (void)cxx14::TaskRunner::global().start( //
            std::move(impl_), std::move(fn));
    }

    // FIXME: ability to print events on the standard error?

#undef MK_NETTESTS_CALL_AND_SUPPRESS // Tidy

  protected:
    // Implementation note: using a SharedPtr<T> because it's easy to
    // move around (especially into lambdas) and because it provides the
    // guarantee of throwing on null, which was a trait of the previous
    // implementation of the nettests API.
    //
    // Also: the pointer was public in the previous implementation but
    // it was also opaque, so not very useful. For this reason, it's
    // now protected in this implementation.
    //
    // FIXME: no longer the case. Also change the variable name.
    cxx14::TaskInfo impl_;
};

#define MK_DECLARE_TEST(_name_, _type_ignored_, _mandatory_ignored_)           \
    class _name_##Test : public BaseTest {                                     \
      public:                                                                  \
        _name_##Test() : BaseTest() { impl_.name = #_name_; }                  \
    };
MK_ENUM_TASKS(MK_DECLARE_TEST)
#undef MK_DECLARE_TEST // Tidy

} // namespace nettests
} // namespace mk
#endif
