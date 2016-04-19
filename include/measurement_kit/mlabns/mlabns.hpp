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

/// Query for mlab-ns.
class Query {
  public:
    std::string policy;         ///< 'geo' | 'random' | 'metro' | 'country'
    std::string metro;          ///< e.g. 'trn'
    std::string address_family; ///< 'ipv4' | 'ipv6'

    Query() {}               ///< Default constructor
    Query(std::nullptr_t) {} ///< Constructor with null params

    /// Construct object from the specified settings.
    /// \param settings Initializer list containing settings.
    /// \remark No exception is raised if settings are invalid. You will
    //          notice that later when the query string is created.
    /// Example:
    ///     Query request({
    ///         {"policy", "random"},
    ///         {"metro, "trn"},
    ///         {"address_family", "ipv6"}
    ///     });
    Query(std::initializer_list<std::pair<std::string, std::string>> settings);

    /// Obtain query from parameters.
    /// \return Query string on success, otherwise error.
    ErrorOr<std::string> as_query();
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
/// \param tool Name of tool (e.g. 'ndt', 'neubot').
/// \param callback Callback called on response or error.
/// \param request Optional request parameters.
void query(std::string tool, Callback<Reply> callback,
           Query request = nullptr);

} // namespace mlabns
} // namespace mk
#endif
