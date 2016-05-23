// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_REPORT_BASE_REPORTER_HPP
#define SRC_REPORT_BASE_REPORTER_HPP

#include <ctime>
#include <measurement_kit/common.hpp>
#include <measurement_kit/ext.hpp>
#include "src/common/utils.hpp"

using json = nlohmann::json;

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

    BaseReporter() {}

    virtual ~BaseReporter() {}

    virtual void open();

    virtual void write_entry(json &entry);

    virtual void close();

    void on_error(Delegate<Error> func) { error_fn_ = func; }

    void emit_error(Error err) {
        if (!error_fn_) {
            throw err;
        }
        error_fn_(err);
    }

  private:
    Delegate<Error> error_fn_;
    bool closed_ = false;
    bool openned_ = false;
};

} // namespace report
} // namespace mk
#endif
