// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_NET_TEST_HPP
#define MEASUREMENT_KIT_NETTESTS_NET_TEST_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace nettests {

class NetTest {
  public:
    NetTest &on_log(Delegate<uint32_t, const char *>);
    NetTest &set_verbosity(uint32_t);
    NetTest &increase_verbosity();
    virtual void begin(Callback<Error>);
    virtual void end(Callback<Error>);

    NetTest();
    NetTest(Settings);
    NetTest(std::string, Settings);
    virtual ~NetTest();

    NetTest &set_input_filepath(std::string);
    NetTest &set_output_filepath(std::string);
    NetTest &set_error_filepath(std::string);
    NetTest &set_reactor(Var<Reactor>);

    template <typename T> NetTest &set_options(std::string key, T value) {
        options[key] = value;
        return *this;
    }

    NetTest &on_entry(Delegate<std::string>);
    NetTest &on_begin(Delegate<>);
    NetTest &on_end(Delegate<> cb);

    virtual Var<NetTest> create_test_();

    void run();
    void run(Callback<> func);

    Var<Logger> logger = Logger::make();
    Var<Reactor> reactor = Reactor::global();
    Settings options;
    std::string input_filepath;
    std::string output_filepath;
    Delegate<std::string> entry_cb;
    Delegate<> begin_cb;
    Delegate<> end_cb;
};

} // namespace nettests
} // namespace mk
#endif
