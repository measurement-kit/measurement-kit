// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_REPORT_REPORT_HPP
#define MEASUREMENT_KIT_REPORT_REPORT_HPP

#include <ctime>
#include <measurement_kit/common.hpp>
#include <measurement_kit/report/base_reporter.hpp>
#include <measurement_kit/report/entry.hpp>
#include <vector>

namespace mk {
namespace report {

class Report {
  public:
    const std::string software_name = "measurement_kit";
    const std::string software_version = MEASUREMENT_KIT_VERSION;
    const std::string data_format_version = "0.2.0";

    std::string test_name;
    std::string test_version;

    std::string probe_ip;
    std::string probe_asn;
    std::string probe_cc;

    tm test_start_time;

    Settings options;

    Report();

    void add_reporter(Var<BaseReporter> reporter);

    void fill_entry(Entry &entry);

    void open(Callback<Error> callback);

    void write_entry(Entry &entry, Callback<Error> callback);

    void close(Callback<Error> callback);

  private:
    std::vector<Var<BaseReporter>> reporters_;
    bool closed_ = false;
    bool openned_ = false;
};

} // namespace report
} // namespace mk
#endif
