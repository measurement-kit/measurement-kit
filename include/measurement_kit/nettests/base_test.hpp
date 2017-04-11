// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_BASE_TEST_HPP
#define MEASUREMENT_KIT_NETTESTS_BASE_TEST_HPP

#include <measurement_kit/nettests/runnable.hpp>

namespace mk {
namespace nettests {
class Runnable;

class BaseTest {
  public:
    BaseTest &on_logger_eof(Delegate<>);
    BaseTest &on_log(Delegate<uint32_t, const char *>);
    BaseTest &on_event(Delegate<const char *>);
    BaseTest &on_progress(Delegate<double, const char *>);
    BaseTest &set_verbosity(uint32_t);
    BaseTest &increase_verbosity();

    BaseTest();
    virtual ~BaseTest();

    BaseTest &add_input_filepath(std::string);
    BaseTest &set_input_filepath(std::string);
    BaseTest &set_output_filepath(std::string);
    BaseTest &set_error_filepath(std::string);

    template <typename T> BaseTest &set_options(std::string key, T value) {
        runnable->options[key] = value;
        return *this;
    }

    BaseTest &on_entry(Delegate<std::string>);
    BaseTest &on_begin(Delegate<>);
    BaseTest &on_end(Delegate<> cb);
    BaseTest &on_destroy(Delegate<> cb);

    void run();
    void start(Callback<> func);

    Var<Runnable> runnable;
};

} // namespace nettests
} // namespace mk
#endif
