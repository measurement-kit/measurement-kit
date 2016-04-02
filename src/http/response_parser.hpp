// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_HTTP_RESPONSE_PARSER_HPP
#define SRC_HTTP_RESPONSE_PARSER_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <measurement_kit/http.hpp>

#include <functional>
#include <iosfwd>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace mk {
namespace http {

// TODO: it would probably optimal to merge `ResponseParserImpl` with this
// class because actually here we have just code duplication after refactoring
// that simplified the logic of `ResponseParserImpl`.

class ResponseParserImpl; // Forward declaration

class ResponseParser : public NonCopyable, public NonMovable {
  public:
    ResponseParser(Logger * = Logger::global());
    ~ResponseParser();

    void on_begin(std::function<void()> &&fn);

    void on_headers_complete(
        std::function<void(unsigned short http_major, unsigned short http_minor,
                           unsigned int status_code, std::string &&reason,
                           Headers &&headers)> &&fn);

    void on_body(std::function<void(std::string &&)> &&fn);

    void on_end(std::function<void()> &&fn);

    void feed(net::Buffer &data);

    void feed(std::string data);

    void feed(char c);

    void eof();

  protected:
    ResponseParserImpl *impl = nullptr;
};

} // namespace http
} // namespace mk
#endif
