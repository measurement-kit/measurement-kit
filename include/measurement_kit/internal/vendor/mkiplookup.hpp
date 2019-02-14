// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKIPLOOKUP_HPP
#define MEASUREMENT_KIT_MKIPLOOKUP_HPP

#include <stdint.h>

#include <string>
#include <vector>

namespace mk {
namespace iplookup {

/// Request is a IP lookup request.
struct Request {
  /// ca_bundle_path is the path of the CA bundle to use.
  std::string ca_bundle_path;

  /// timeout is the timeout in seconds.
  int64_t timeout = 30;
};

/// Response is a IP lookup response.
struct Response {
  /// good indicates whether we succeeded.
  bool good = false;

  /// logs contains (possibly non UTF-8) logs.
  std::vector<std::string> logs;

  /// probe_ip is the probe IP.
  std::string probe_ip;

  /// bytes_sent is the bytes sent in the request.
  int64_t bytes_sent = 0;

  /// bytes_recv is the bytes recv in the response.
  int64_t bytes_recv = 0;
};

/// perform performs an IP lookup using @p request settings.
Response perform(const Request &request) noexcept;

}  // namespace iplookup
}  // namespace mk

// By default the implementation is not included. You can force it being
// included by providing the following definition to the compiler.
//
// If you're just into understanding the API, you can stop reading here.
#ifdef MKIPLOOKUP_INLINE_IMPL

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#endif

#include <algorithm>
#include <memory>
#include <sstream>

#include "mkdata.hpp"
#include "mkcurl.hpp"
#include "mkmock.hpp"

// MKIPLOOKUP_MOCK allows to enable mocking
#ifdef MKIPLOOKUP_MOCK
#define MKIPLOOKUP_HOOK MKMOCK_HOOK_ENABLED
#define MKIPLOOKUP_HOOK_ALLOC MKMOCK_HOOK_ALLOC_ENABLED
#else
#define MKIPLOOKUP_HOOK MKMOCK_HOOK_DISABLED
#define MKIPLOOKUP_HOOK_ALLOC MKMOCK_HOOK_ALLOC_DISABLED
#endif

namespace mk {
namespace iplookup {

// ubuntu_get_url returns the URL to perform a IP lookup using
// the GeoIP services provided by Ubuntu.
static const char *ubuntu_get_url() noexcept {
  return "https://geoip.ubuntu.com/lookup";
}

// ubuntu_extract extracts a candidate @p probe_ip from @p body. Returns
// true on success, false on failure. Note that you still need to parse
// the returned @p probe_ip to make sure it's a valid IP address.
static bool ubuntu_extract(std::string body, std::string &probe_ip) noexcept {
  probe_ip = "";  // reset
  if (!mk::data::contains_valid_utf8(body)) {
    return false;
  }
  static const std::string open_tag = "<Ip>";
  static const std::string close_tag = "</Ip>";
  auto pos = body.find(open_tag);
  if (pos == std::string::npos) {
    return false;
  }
  body = body.substr(pos + open_tag.size());  // Find EOS in the worst case
  pos = body.find(close_tag);
  if (pos == std::string::npos) {
    return false;
  }
  body = body.substr(0, pos);
  for (char ch : body) {
    if (ch == ' ' || ch == '\t') {
      continue;
    }
    auto ok = isdigit(ch) || (ch >= 'a' && ch <= 'f') ||  //
              (ch >= 'A' && ch <= 'F') || ch == '.' || ch == ':';
    if (!ok) {
      return false;
    }
    probe_ip += ch;
  }
  return true;
}

// mkiplookup_ainfop_deleter is a deleter for `addrinfo **`.
struct ainfop_deleter {
  void operator()(addrinfo **infop) {
    if (infop != nullptr) {
      freeaddrinfo(*infop);  // https://github.com/lh3/samtools/issues/21
    }
    delete infop;
  }
};

Response perform(const Request &request) noexcept {
  mk::curl::Request r;
  r.timeout = request.timeout;
  r.ca_path = request.ca_bundle_path;
  r.url = ubuntu_get_url();
  mk::curl::Response re = mk::curl::perform(r);
  Response response;
  response.bytes_recv = re.bytes_recv;
  response.bytes_sent = re.bytes_sent;
  for (auto &log : re.logs) {
    response.logs.push_back(std::move(log.line));
  }
  MKIPLOOKUP_HOOK(mkcurl_response_error, re.error);
  if (re.error != 0) {
    response.logs.push_back("We could not communicate with server.");
    return response;
  }
  MKIPLOOKUP_HOOK(mkcurl_response_status_code, re.status_code);
  if (re.status_code != 200) {
    response.logs.push_back("Status code indicates failure.");
    return response;
  }
  response.logs.push_back("=== BEGIN RECEIVED BODY ===");
  response.logs.push_back(re.body);  // makes a copy
  response.logs.push_back("=== END RECEIVED BODY ===");
  std::string maybe_probe_ip;
  bool matches = ubuntu_extract(std::move(re.body), maybe_probe_ip);
  MKIPLOOKUP_HOOK(ubuntu_extract_result, matches);
  if (!matches) {
    response.logs.push_back("Cannot extract IP from response body.");
    return response;
  }
  // Make sure the candidate probe_ip is a _real_ IP address.
  {
    addrinfo hints{};
    hints.ai_flags |= AI_NUMERICHOST | AI_NUMERICSERV;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    std::unique_ptr<addrinfo *, ainfop_deleter> ap{new addrinfo *{}};
    // Note that _any_ port would do in this context.
    int rv = getaddrinfo(maybe_probe_ip.c_str(), "443", &hints, ap.get());
    MKIPLOOKUP_HOOK(getaddrinfo_retval, rv);
    if (rv != 0) {
      std::stringstream ss;
      ss << "Not a valid IP address: ";
      ss << gai_strerror(rv);
      response.logs.push_back(ss.str());
      return response;
    }
  }
  std::swap(response.probe_ip, maybe_probe_ip);
  response.good = true;
  return response;
}

}  // namespace iplookup
}  // namespace mk
#endif  // MKIPLOOKUP_INLINE_IMPL
#endif  // MEASUREMENT_KIT_MKIPLOOKUP_HPP
