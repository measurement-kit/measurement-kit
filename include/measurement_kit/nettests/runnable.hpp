// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_RUNNABLE_HPP
#define MEASUREMENT_KIT_NETTESTS_RUNNABLE_HPP

#include <measurement_kit/report.hpp>

#include <ctime>
#include <deque>
#include <list>
#include <sstream>

namespace mk {
namespace nettests {

class Runnable : public NonCopyable, public NonMovable {
  public:
    void begin(Callback<Error>);
    void end(Callback<Error>);

    virtual ~Runnable();

    Var<Logger> logger = Logger::make();
    Var<Reactor> reactor;  /* Left unspecified by purpose */
    Settings options;
    std::list<std::string> input_filepaths;
    std::string output_filepath;
    Delegate<std::string> entry_cb;
    Delegate<> begin_cb;
    std::list<Delegate<>> end_cbs;
    std::list<Delegate<>> destroy_cbs;
    bool needs_input = false;
    bool use_bouncer = true;
    std::map<std::string, std::string> test_helpers_data;

    std::string test_name = "ooni_test";
    std::string test_version = "0.0.1";
    std::string probe_ip = "127.0.0.1";
    std::string probe_asn = "AS0";
    std::string probe_cc = "ZZ";
    std::string resolver_ip = "127.0.0.1";

  protected:
    // Functions that derived classes SHOULD override
    virtual void setup(std::string);
    virtual void teardown(std::string);
    virtual void main(std::string, Settings, Callback<Var<report::Entry>>);
    virtual void fixup_entry(report::Entry &);

    // Functions that derived classes should access
    std::list<std::string> test_helpers_option_names();
    std::list<std::string> test_helpers_bouncer_names();

  private:
    report::Report report;
    tm test_start_time;
    std::deque<std::string> inputs;
    double beginning = 0.0;

    void run_next_measurement(size_t, Callback<Error>, size_t, Var<size_t>);
    void contact_bouncer(Callback<Error>);
    void geoip_lookup(Callback<>);
    void open_report(Callback<Error>);
    std::string generate_output_filepath();
};

} // namespace nettests
} // namespace mk
#endif
