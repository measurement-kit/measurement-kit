// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_TEST_HPP
#define MEASUREMENT_KIT_OONI_TEST_HPP

#include <ctime>
#include <measurement_kit/report.hpp>
#include <sstream>

namespace mk {
namespace ooni {

class OoniTest : public NetTest, public NonCopyable, public NonMovable {
    // Note: here we make the reasonable assumption that the owner of this
    // instance would keep it safe until the final callback is fired

  public:
    std::string test_name;
    std::string test_version;
    std::string probe_ip = "127.0.0.1";
    std::string probe_asn = "AS0";
    std::string probe_cc = "ZZ";
    bool needs_input = false;

    OoniTest() : OoniTest("", Settings()) {}

    virtual ~OoniTest() {}

    OoniTest(std::string f) : OoniTest(f, Settings()) {}

    OoniTest(std::string f, Settings o) : NetTest(f, o),
        test_name("net_test"), test_version("0.0.1") {}

    void begin(Callback<>) override;
    void end(Callback<>) override;

  protected:
    // Functions that derived classes SHOULD override
    virtual void setup(std::string) {}
    virtual void teardown(std::string) {}
    virtual void main(std::string, Settings, Callback<report::Entry> cb) {
        reactor->call_soon([=]() { cb(report::Entry{}); });
    }

  private:
    report::FileReporter file_report;
    tm test_start_time;
    Var<std::istream> input_generator;

    void run_next_measurement(Callback<>);
    void geoip_lookup(Callback<>);
    void open_report();
    std::string generate_output_filepath();
};

} // namespace mk
} // namespace ooni
#endif
