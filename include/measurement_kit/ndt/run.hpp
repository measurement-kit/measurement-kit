// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_RUN_HPP
#define MEASUREMENT_KIT_NDT_RUN_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace ndt {

// By default we pass MK_NDT_UPLOAD|MK_NDT_DOWNLOAD as settings["test_suite"]
// but you can tweak that by only requesting a single phase.
#define MK_NDT_MIDDLEBOX 1
#define MK_NDT_UPLOAD 2
#define MK_NDT_DOWNLOAD 4
#define MK_NDT_SIMPLE_FIREWALL 8
#define MK_NDT_STATUS 16
#define MK_NDT_META 32
#define MK_NDT_UPLOAD_EXT 64 // not implemented
#define MK_NDT_DOWNLOAD_EXT 128

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
