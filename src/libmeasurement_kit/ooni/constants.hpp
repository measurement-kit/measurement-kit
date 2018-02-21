// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_CONSTANTS_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_CONSTANTS_HPP

#include "src/libmeasurement_kit/http/http.hpp"

#include <set>

namespace mk {
namespace ooni {
namespace constants {

// These are very common server headers that we don't consider when checking
// between control and experiment.
const std::set<std::string> COMMON_SERVER_HEADERS = {
  "date",
  "content-type",
  "server",
  "cache-control",
  "vary",
  "set-cookie",
  "location",
  "expires",
  "x-powered-by",
  "content-encoding",
  "last-modified",
  "accept-ranges",
  "pragma",
  "x-frame-options",
  "etag",
  "x-content-type-options",
  "age",
  "via",
  "p3p",
  "x-xss-protection",
  "content-language",
  "cf-ray",
  "strict-transport-security",
  "link",
  "x-varnish"
};


const http::Headers COMMON_CLIENT_HEADERS = {
  {
    "User-Agent",
    "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36"
  },
  {
    "Accept-Language", "en-US;q=0.8,en;q=0.5"
  },
  {
    "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"
  }
};

const std::string MEEK_SERVER_RESPONSE = "I\xe2\x80\x99m just a happy little web server.\n";

static std::vector<std::string> COMMON_USER_AGENTS = {
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1.7) Gecko/20091221 Firefox/3.5.7",
    "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3 1 2 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Mobile/7D11",
    "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; de; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 6.1; de; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; de; rv:1.9.2) Gecko/20100115 Firefox/3.6",
    "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.7) Gecko/20091221 Firefox/3.5.7",
    "Mozilla/5.0 (Windows; U; Windows NT 5.1; de; rv:1.9.1.7) Gecko/20091221 Firefox/3.5.7 (.NET CLR 3.5.30729))",
};


} // namespace constants
} // namespace ooni
} // namespace mk

#endif
