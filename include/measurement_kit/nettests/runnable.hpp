// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_RUNNABLE_HPP
#define MEASUREMENT_KIT_NETTESTS_RUNNABLE_HPP

#include <measurement_kit/report.hpp>

#include <ctime>
#include <sstream>

namespace mk {
namespace nettests {

class Runnable : public NonCopyable, public NonMovable {
  public:
    void begin(Callback<Error>);
    void end(Callback<Error>);

    virtual ~Runnable();

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

  protected:
    // Functions that derived classes SHOULD override
    virtual void setup(std::string);
    virtual void teardown(std::string);
    virtual void main(std::string, Settings, Callback<report::Entry>);

  private:
    report::Report report;
    tm test_start_time;
    Var<std::istream> input_generator;

    void run_next_measurement(size_t, Callback<Error>, size_t, Var<size_t>);
    void geoip_lookup(Callback<>);
    void open_report(Callback<Error>);
    std::string generate_output_filepath();
};

} // namespace nettests
} // namespace mk
#endif
