// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_MLABNS_MLABNS_HPP
#define MEASUREMENT_KIT_MLABNS_MLABNS_HPP

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <measurement_kit/common.hpp>
#include <string>
#include <utility>
#include <vector>

namespace mk {
namespace mlabns {

MK_DEFINE_ERR(MK_ERR_MLABNS(0), InvalidPolicyError, "")
MK_DEFINE_ERR(MK_ERR_MLABNS(1), InvalidAddressFamilyError, "")
MK_DEFINE_ERR(MK_ERR_MLABNS(2), InvalidMetroError, "")
MK_DEFINE_ERR(MK_ERR_MLABNS(3), InvalidToolNameError, "")

/// Reply to mlab-ns query.
class Reply {
  public:
    std::string city;            ///< City where sliver is.
    std::string url;             ///< URL to access sliver using HTTP.
    std::vector<std::string> ip; ///< List of IP addresses of sliver.
    std::string fqdn;            ///< FQDN of sliver.
    std::string site;            ///< Site where sliver is.
    std::string country;         ///< Country where sliver is.
};

/// Query mlab-ns and receive response.
void query(std::string tool, Callback<Error, Reply> callback,
           Settings settings = {}, Var<Reactor> reactor = Reactor::global(),
           Var<Logger> logger = Logger::global());

} // namespace mlabns
} // namespace mk
#endif
