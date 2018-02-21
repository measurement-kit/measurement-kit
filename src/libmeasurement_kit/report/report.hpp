// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_REPORT_REPORT_HPP
#define SRC_LIBMEASUREMENT_KIT_REPORT_REPORT_HPP

#include <measurement_kit/common.hpp>

#include <ctime>

#include "src/libmeasurement_kit/report/entry.hpp"

namespace mk {
namespace report {

class BaseReporter; // Forward decl.

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

    std::string report_id; /* Set after open(), if possible */

    Report();

    void add_reporter(SharedPtr<BaseReporter> reporter);

    void fill_entry(Entry &entry) const;

    Entry get_dummy_entry() const;

    void open(Callback<Error> callback);

    void write_entry(Entry entry, Callback<Error> callback, SharedPtr<Logger> logger);

    void close(Callback<Error> callback);

  private:
    std::vector<SharedPtr<BaseReporter>> reporters_;
};

} // namespace report
} // namespace mk
#endif
