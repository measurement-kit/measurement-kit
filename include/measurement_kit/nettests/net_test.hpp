// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_NET_TEST_HPP
#define MEASUREMENT_KIT_NETTESTS_NET_TEST_HPP

#include <measurement_kit/report.hpp>

#include <ctime>
#include <sstream>

namespace mk {
namespace nettests {

class NetTest : public NonCopyable, public NonMovable {
    // Note: here we make the reasonable assumption that the owner of this
    // instance would keep it safe until the final callback is fired

  public:
    NetTest &on_log(Delegate<uint32_t, const char *>);
    NetTest &set_verbosity(uint32_t);
    NetTest &increase_verbosity();

    // Both implemented in ooni_test.cpp
    void begin(Callback<Error>);
    void end(Callback<Error>);

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

    std::string test_name;
    std::string test_version;
    std::string probe_ip = "127.0.0.1";
    std::string probe_asn = "AS0";
    std::string probe_cc = "ZZ";
    std::string resolver_ip = "127.0.0.1";
    bool needs_input = false;

    // Everything from here on implemented in ooni_test.cpp:

  protected:
    // Functions that derived classes SHOULD override
    virtual void setup(std::string) {}
    virtual void teardown(std::string) {}
    virtual void main(std::string, Settings, Callback<report::Entry> cb) {
        reactor->call_soon([=]() { cb(report::Entry{}); });
    }

  private:
    report::Report report;
    tm test_start_time;
    Var<std::istream> input_generator;

    void run_next_measurement(size_t, Callback<Error>, size_t,
                              Var<size_t>);
    void geoip_lookup(Callback<>);
    void open_report(Callback<Error>);
    std::string generate_output_filepath();
};

} // namespace nettests
} // namespace mk
#endif
