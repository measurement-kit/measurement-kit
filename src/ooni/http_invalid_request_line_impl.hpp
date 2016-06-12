// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_HTTP_INVALID_REQUEST_LINE_HPP
#define SRC_OONI_HTTP_INVALID_REQUEST_LINE_HPP

#include <measurement_kit/http.hpp>
#include <measurement_kit/ooni.hpp>

#include "src/common/utils.hpp"
#include <sys/stat.h>

namespace mk {
namespace ooni {

class HTTPInvalidRequestLineImpl : public OoniTest {
  public:
    HTTPInvalidRequestLineImpl(Settings options_) : OoniTest("", options_) {
        test_name = "http_invalid_request_line";
        test_version = "0.0.1";
    }

    void main(std::string, Settings options, Callback<report::Entry> cb) {
        http_invalid_request_line(options, [=](Var<report::Entry> e) {
             cb(*e);
        }, reactor, logger);
    }
};

} // namespace ooni
} // namespace mk
#endif
