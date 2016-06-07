// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_REPORT_BASE_REPORTER_HPP
#define MEASUREMENT_KIT_REPORT_BASE_REPORTER_HPP

#include <ctime>
#include <measurement_kit/common.hpp>
#include <measurement_kit/report/entry.hpp>

namespace mk {
namespace report {

class BaseReporter {
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

    BaseReporter();

    virtual ~BaseReporter() {}

#define XX __attribute__((warn_unused_result))

    virtual Error open() XX;

    virtual Error write_entry(Entry &entry) XX;

    virtual Error close() XX;

#undef XX

  private:
    bool closed_ = false;
    bool openned_ = false;
};

} // namespace report
} // namespace mk
#endif
