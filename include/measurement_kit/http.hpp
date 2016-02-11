// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_HTTP_HPP
#define MEASUREMENT_KIT_HTTP_HPP

#include <functional>
#include <map>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/settings.hpp>
#include <set>
#include <string>

namespace mk {
namespace http {

/// Received UPGRADE request error.
class UpgradeError : public Error {
  public:
    UpgradeError (): Error(3000, "unknown_error 3000") {};
};

/// Parse error occurred.
class ParserError : public Error {
  public:
    ParserError() : Error(3001, "unknown_error 3001") {};
};


/// Url Parse error occurred.
class UrlParserError : public Error {
  public:
    UrlParserError() : Error(3002, "unknown_error 3002") {};
};

/// Missing Url Schema error occurred.
class MissingUrlSchemaError : public Error {
  public:
    MissingUrlSchemaError() : Error(3003, "unknown_error 3003") {};
};

/// Missing Url Host error occurred.
class MissingUrlHostError : public Error {
  public:
    MissingUrlHostError() : Error(3004, "unknown_error 3004") {};
};

/// HTTP headers.
typedef std::map<std::string, std::string> Headers;

/// HTTP response.
struct Response {
    std::string response_line; ///< Original HTTP response line.
    unsigned short http_major; ///< HTTP major version number.
    unsigned short http_minor; ///< HTTP minor version number.
    unsigned int status_code;  ///< HTTP status code.
    std::string reason;        ///< HTTP reason string.
    Headers headers;           ///< Response headers.
    std::string body;          ///< Response body.
};

/// Type of callback called on error or when response is complete.
typedef std::function<void(Error, Response)> RequestCallback;

// Forward declaration of internally used class.
class Request;

/// HTTP client.
class Client {

  protected:
    std::set<Request *> pending;

  public:
    /// Issue HTTP request.
    void request(Settings settings, Headers headers, std::string body,
                 RequestCallback callback, Logger *lp = Logger::global());

    Client() {} ///< Default constructor
    ~Client();  ///< Destructor

    //
    // TODO: implement all the fancy methods
    //
};

} // namespace http
} // namespace mk
#endif
