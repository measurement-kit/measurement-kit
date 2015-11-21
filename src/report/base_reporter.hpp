// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_REPORT_BASE_REPORTER_HPP
#define MEASUREMENT_KIT_REPORT_BASE_REPORTER_HPP

#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/version.hpp>
#include "src/report/entry.hpp"

namespace measurement_kit {
namespace report {

class BaseReporter {
  public:
    std::string test_name;
    std::string test_version;
    std::string probe_ip;

    std::string probe_asn;
    std::string probe_cc;

    time_t start_time;

    common::Settings options;

    BaseReporter(){};

    virtual ~BaseReporter(){};

    std::string getHeader();

    virtual void open();

    virtual void writeEntry(Entry &entry);

    virtual void close();

    void on_error(std::function<void(common::Error)> func) {
        error_fn_ = func;
    }

    void emit_error(common::Error err) {
        if (!error_fn_) throw err;
        error_fn_(err);
    }

  private:
    std::function<void(common::Error)> error_fn_;
    bool closed = false;
    bool openned = false;

    const std::string software_name = "measurement_kit";
    const std::string software_version = MEASUREMENT_KIT_VERSION;
    const std::string data_format_version = "0.1";
};

} // namespace report
} // namespace measurement_kit
#endif
