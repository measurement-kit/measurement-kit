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

/// Invalid mlab-ns policy error
class InvalidPolicyError : public Error {
  public:
    /// Default constructor
    InvalidPolicyError() : Error(5000, "unknown_failure 5000") {}
};

/// Invalid mlab-ns address-family error
class InvalidAddressFamilyError : public Error {
  public:
    /// Default constructor
    InvalidAddressFamilyError() : Error(5001, "unknown_failure 5001") {}
};

/// Invalid mlab-ns metro error
class InvalidMetroError : public Error {
  public:
    /// Default constructor
    InvalidMetroError() : Error(5002, "unknown_failure 5002") {}
};

/// Invalid mlab-ns tool name error
class InvalidToolNameError : public Error {
  public:
    /// Default constructor
    InvalidToolNameError() : Error(5003, "unknown_failure 5003") {}
};

/// Invalid mlab-ns HTTP status code error
class UnexpectedHttpStatusCodeError : public Error {
  public:
    /// Default constructor
    UnexpectedHttpStatusCodeError() : Error(5004, "unknown_failure 5004") {}
};

/// Invalid mlab-ns JSON error
class JsonParsingError : public Error {
  public:
    /// Default constructor
    JsonParsingError() : Error(5005, "unknown_failure 5005") {}
};

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
