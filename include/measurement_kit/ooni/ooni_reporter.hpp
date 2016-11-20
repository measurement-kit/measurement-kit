// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_OONI_REPORTER_HPP
#define MEASUREMENT_KIT_OONI_OONI_REPORTER_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {

class OoniReporter : public report::BaseReporter {
  public:
    static Var<BaseReporter> make(Settings, Var<Reactor>, Var<Logger>);

    Continuation<Error> open(report::Report report) override;
    Continuation<Error> write_entry(report::Entry entry) override;
    Continuation<Error> close() override;

    ~OoniReporter() override {}

  private:
    OoniReporter(Settings, Var<Reactor>, Var<Logger>);

    Var<Reactor> reactor = Reactor::global();
    Var<Logger> logger = Logger::global();
    Settings settings; // Our private copy of the ooni_test settings
    std::string report_id;
};

} // namespace ooni
} // namespace mk
#endif
