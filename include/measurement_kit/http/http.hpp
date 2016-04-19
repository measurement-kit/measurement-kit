// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_HTTP_HTTP_HPP
#define MEASUREMENT_KIT_HTTP_HTTP_HPP

#include <functional>
#include <map>
#include <measurement_kit/common.hpp>
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

class MissingUrlError : public Error {
  public:
    MissingUrlError() : Error(3005, "unknown_error 3005") {};
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
typedef Callback<Response> RequestCallback;

// Forward declaration of internally used class.
class Request;

/// Send HTTP request and receive response.
/// \param settings Settings for HTTP request.
/// \param cb Callback called when complete or on error.
/// \param headers Optional HTTP request headers.
/// \param body Optional HTTP request body.
/// \param lp Optional logger.
/// \param pol Optional poller.
void request(Settings settings, RequestCallback cb, Headers headers = {},
             std::string body = "", Logger *lp = Logger::global(),
             Poller *pol = Poller::global());

// Signature of the old http::Client ->request method, widely used
inline void request(Settings settings, Headers headers, std::string body,
        RequestCallback cb, Logger *lp = Logger::global(),
        Poller *pol = Poller::global()) {
    request(settings, cb, headers, body, lp, pol);
}

/// Represents a URL.
class Url {
  public:
    std::string schema;    /// URL schema
    std::string address;   /// URL address
    int port = 80;         /// URL port
    std::string path;      /// URL path
    std::string query;     /// URL query
    std::string pathquery; /// URL path followed by optional query
};

/// Parses a URL.
/// \param url Input URL you want to parse.
/// \return The parsed URL.
/// \throw Exception on failure.
Url parse_url(std::string url);

/// Parses a URL without throwing an exception on failure.
/// \param url Input URL you want to parse..
/// \return An error (on failure) or the parsed URL.
ErrorOr<Url> parse_url_noexcept(std::string url);

/// Send HTTP GET and receive response.
/// \param url URL to send request to.
/// \param settings Settings for HTTP request.
/// \param cb Callback called when complete or on error.
/// \param headers Optional HTTP request headers.
/// \param body Optional HTTP request body.
/// \param lp Optional logger.
/// \param pol Optional poller.
inline void get(std::string url, RequestCallback cb,
                Headers headers = {}, std::string body = "",
                Settings settings = {}, Logger *lp = Logger::global(),
                Poller *pol = Poller::global()) {
    settings["method"] = "GET";
    settings["url"] = url;
    request(settings, cb, headers, body, lp, pol);
}

/// Send HTTP request and receive response.
/// \param method Method to use.
/// \param url URL to send request to.
/// \param settings Settings for HTTP request.
/// \param cb Callback called when complete or on error.
/// \param headers Optional HTTP request headers.
/// \param body Optional HTTP request body.
/// \param lp Optional logger.
/// \param pol Optional poller.
inline void request(std::string method, std::string url, RequestCallback cb,
                    Headers headers = {}, std::string body = "",
                    Settings settings = {}, Logger *lp = Logger::global(),
                    Poller *pol = Poller::global()) {
    settings["method"] = method;
    settings["url"] = url;
    request(settings, cb, headers, body, lp, pol);
}

} // namespace http
} // namespace mk
#endif
