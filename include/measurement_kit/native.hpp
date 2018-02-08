// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NATIVE_HPP
#define MEASUREMENT_KIT_NATIVE_HPP

/*
 * Measurement Kit interface for native (i.e. C++) users.
 *
 * See include/measurement_kit/engine.h for more documentation.
 *
 * See example/native/ndt.cpp for example.
 */

#include <measurement_kit/swig.hpp>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

#include <measurement_kit/common/nlohmann/json.hpp>

namespace mk {
namespace native {

class BaseTask {
  public:
    // constructor and destructor

    explicit BaseTask(const std::string &name) {
        settings_ = nlohmann::json{
                {"annotations", nlohmann::json::array()},
                {"enabled_events", nlohmann::json::array()},
                {"inputs", nlohmann::json::array()},
                {"input_files", nlohmann::json::array()},
                {"log_file", nullptr},
                {"output_file", nullptr},
                {"options", nlohmann::json::object()},
                {"type", name},
                {"verbosity", "QUIET"},
        };
    }

    ~BaseTask() {}

    // configuration of fields

    void add_annotation(const std::string &key, int64_t value) {
        settings_.at("annotations")[key] = value;
    }
    void add_annotation(const std::string &key, double value) {
        settings_.at("annotations")[key] = value;
    }
    void add_annotation(const std::string &key, const std::string &value) {
        settings_.at("annotations")[key] = value;
    }

    void add_input(const std::string &input) {
        settings_.at("inputs").push_back(input);
    }
    void add_input_file(const std::string &path) {
        settings_.at("input_files").push_back(path);
    }

    void set_log_file(const std::string &path) {
        settings_.at("log_file") = path;
    }

    void set_option(const std::string &key, int64_t value) {
        settings_.at("options")[key] = value;
    }
    void set_option(const std::string &key, double value) {
        settings_.at("options")[key] = value;
    }
    void set_option(const std::string &key, const std::string &value) {
        settings_.at("options")[key] = value;
    }

    void set_output_file(const std::string &path) {
        settings_.at("output_file") = path;
    }

    void set_verbosity(const std::string &verbosity) {
        settings_.at("verbosity") = verbosity;
    }

    // configuration of callbacks

    void on_failure(Callback<const std::string &> &&cb) {
        settings_.at("enabled_events").push_back("FAILURE");
        std::swap(failure_cb_, callback);
    }

    void on_log(Callback<const std::string &, const std::string &> &&cb) {
        settings_.at("enabled_events").push_back("LOG");
        std::swap(log_cb_, callback);
    }

    void on_performance(
            Callback<const std::string &, double, int, double> &&cb) {
        settings_.at("enabled_events").push_back("PERFORMANCE");
        std::swap(performance_cb_, callback);
    }

    void on_progress(Callback<double, const std::string &> &&cb) {
        settings_.at("enabled_events").push_back("PROGRESS");
        std::swap(progress_cb_, callback);
    }

    void on_result(Callback<const nlohmann::json &> &&cb) {
        settings_.at("enabled_events").push_back("RESULT");
        std::swap(result_cb_, callback);
    }

    // run

    // XXX we probably also want the possibility to interrupt...
    void run() {
        swig::Task task;
        if (!task.initialize(settings_.dump()) {
            abort(); // should not happen
        }
        do {
            auto s = task.wait_for_next_event();
            auto event = nlohmann::json::parse(s); // we don't expect parse err
            if (event == nullptr) {
                break;
            }
            const auto &type = event.at("type");
            if (type == "FAILURE") {
                failure_cb_(event.at("failure"));
                continue;
            }
            if (type == "LOG") {
                log_cb_(event.at("verbosity"), event.at("message"));
                continue;
            }
            if (type == "PERFORMANCE") {
                performance_cb_(event.at("direction"),
                        event.at("elapsed_seconds"), event.at("num_streams"),
                        event.at("speed_kbit_s"));
                continue;
            }
            if (type == "PROGRESS") {
                progress_cb_(event.at("percentage"), event.at("message"));
                continue;
            }
            if (type == "RESULT") {
                progress_cb_(nlohmann::json::parse(event.at("result")));
                continue;
            }
        } while (1);
    }

  private:
    nlohmann::json settings_;
    Callback<const std::string &> failure_cb_;
    Callback<const std::string &, const std::string &> failure_cb_;
    Callback<const std::string &, double, int, double> performance_cb_;
    Callback<double, const std::string &> progress_cb_;
    Callback<const nlohmann::json &> result_cb_;
};

} // namespace native
} // namespace mk
#endif // MEASUREMENT_KIT_NATIVE_HPP
