// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_OONI_REPORTER_HPP
#define MEASUREMENT_KIT_OONI_OONI_REPORTER_HPP

#include <measurement_kit/report/base_reporter.hpp>

namespace mk {
namespace report {

class OoniReporter : public BaseReporter {
  public:
    static Var<BaseReporter> make(Settings, Reactor, Var<Logger>);

    Continuation<Error> open(Report &report) override;
    Continuation<Error> write_entry(Entry entry) override;
    Continuation<Error> close() override;

    ~OoniReporter() override {}

    std::string get_report_id() override;

  private:
    OoniReporter(Settings, Reactor, Var<Logger>);

    Reactor reactor = Reactor::global();
    Var<Logger> logger = Logger::global();
    Settings settings; // Our private copy of the ooni_test settings
    std::string report_id;
};

} // namespace report
} // namespace mk
#endif
