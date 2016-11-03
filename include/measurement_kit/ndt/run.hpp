// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_RUN_HPP
#define MEASUREMENT_KIT_NDT_RUN_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace ndt {

using namespace mk::report;

// By default we pass MK_NDT_UPLOAD|MK_NDT_DOWNLOAD as settings["test_suite"]
// but you can tweak that by only requesting a single phase.
// XXX These should kept in sync with src/ndt/internal.hpp; perhaps we can
// define all defines here and there just redefine?
#define MK_NDT_UPLOAD 2
#define MK_NDT_DOWNLOAD 4
//#define MK_NDT_UPLOAD_EXT 64 // not implemented
#define MK_NDT_DOWNLOAD_EXT 128

void run_with_specific_server(Var<Entry> entry, std::string address, int port,
                              Callback<Error> callback, Settings settings = {},
                              Var<Logger> logger = Logger::global(),
                              Var<Reactor> reactor = Reactor::global());

void run(Var<Entry> entry, Callback<Error> callback, Settings settings = {},
         Var<Logger> logger = Logger::global(),
         Var<Reactor> reactor = Reactor::global());

} // namespace ndt
} // namespace mk
#endif
