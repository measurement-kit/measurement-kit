// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_TEST_HPP
#define MEASUREMENT_KIT_OONI_TEST_HPP

#include <measurement_kit/report.hpp>

#include <ctime>
#include <sstream>

namespace mk {
namespace ooni {

using namespace mk::report;

class OoniTest : public NetTest, public NonCopyable, public NonMovable {
    // Note: here we make the reasonable assumption that the owner of this
    // instance would keep it safe until the final callback is fired

  public:
    std::string test_name;
    std::string test_version;
    std::string probe_ip = "127.0.0.1";
    std::string probe_asn = "AS0";
    std::string probe_cc = "ZZ";
    std::string resolver_ip = "127.0.0.1";
    bool needs_input = false;

    OoniTest() : OoniTest("", Settings()) {}

    virtual ~OoniTest() {}

    OoniTest(std::string f) : OoniTest(f, Settings()) {}

    OoniTest(std::string f, Settings o) : NetTest(f, o),
        test_name("ooni_test"), test_version("0.0.1") {}

    void begin(Callback<Error>) override;
    void end(Callback<Error>) override;
    void run_with_input(std::string input, Callback<std::string> cb) override {
        main(input, options, [=](report::Entry entry) {
            cb(entry.dump(4));
        });
    }

  protected:
    // Functions that derived classes SHOULD override
    virtual void setup(std::string) {}
    virtual void teardown(std::string) {}
    virtual void main(std::string, Settings, Callback<Entry> cb) {
        reactor->call_soon([=]() { cb(Entry{}); });
    }

  private:
    Report report;
    tm test_start_time;
    Var<std::istream> input_generator;

    void run_next_measurement(size_t, Callback<Error>, size_t,
                              Var<size_t>);
    void geoip_lookup(Callback<>);
    void open_report(Callback<Error>);
    std::string generate_output_filepath();
};

} // namespace mk
} // namespace ooni
#endif
