// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_HTTP_ERROR_HPP
#define MEASUREMENT_KIT_HTTP_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace http {

/// Received UPGRADE request error
class UpgradeError : public Error {
  public:
    UpgradeError (): Error(3000, "unknown_error 3000") {};
};

/// Parse error occurred
class ParserError : public Error {
  public:
    ParserError() : Error(3001, "unknown_error 3001") {};
};


/// Url Parse error occurred
class UrlParserError : public Error {
  public:
    UrlParserError() : Error(3002, "unknown_error 3002") {};
};

/// Missing Url Schema error occurred
class MissingUrlSchemaError : public Error {
  public:
    MissingUrlSchemaError() : Error(3003, "unknown_error 3003") {};
};

/// Missing Url Host error occurred
class MissingUrlHostError : public Error {
  public:
    MissingUrlHostError() : Error(3004, "unknown_error 3004") {};
};

} // namespace http
} // namespace mk
#endif
