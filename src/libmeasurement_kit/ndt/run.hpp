// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NDT_RUN_HPP
#define SRC_LIBMEASUREMENT_KIT_NDT_RUN_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace ndt {

void run_with_specific_server(SharedPtr<report::Entry> entry, std::string address, int port,
                              Callback<Error> callback, Settings settings = {},
                              SharedPtr<Reactor> reactor = Reactor::global(),
                              SharedPtr<Logger> logger = Logger::global());

void run(SharedPtr<report::Entry> entry, Callback<Error> callback, Settings settings = {},
         SharedPtr<Reactor> reactor = Reactor::global(),
         SharedPtr<Logger> logger = Logger::global());

} // namespace ndt
} // namespace mk
#endif
