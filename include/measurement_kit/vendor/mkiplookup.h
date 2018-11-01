// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKIPLOOKUP_H
#define MEASUREMENT_KIT_MKIPLOOKUP_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/// mkiplookup_request_t is an IPLookup request.
typedef struct mkiplookup_request mkiplookup_request_t;

/// mkiplookup_response_t is a IPLookup response.
typedef struct mkiplookup_response mkiplookup_response_t;

/// mkiplookup_request_new_nonnull creates a new mkiplookup_request_t. This
/// function calls abort if the allocation fails, hence it always
/// returns a valid pointer.
mkiplookup_request_t *mkiplookup_request_new_nonnull(void);

/// mkiplookup_request_set_timeout sets the timeout. After the timeout
/// has elapsed, if a response is not received, the request will fail
/// and an error is returned by mkiplookup_request_perform_nonnull. This
/// function calls abort if passed null arguments.
void mkiplookup_request_set_timeout(
    mkiplookup_request_t *request,
    int64_t timeout);

/// mkiplookup_request_set_ca_bundle_path sets the CA bundle path. This
/// is required on mobile. If passed null pointers, this function aborts.
void mkiplookup_request_set_ca_bundle_path(
    mkiplookup_request_t *request,
    const char *ca_bundle_path);

/// mkiplookup_request_perform_nonnull performs a lookup with @p request. This
/// function will never return a null pointer. This function calls abort
/// if you pass a null @p request.
mkiplookup_response_t *mkiplookup_request_perform_nonnull(
    const mkiplookup_request_t *request);

/// mkiplookup_request_delete destroys @p request.
void mkiplookup_request_delete(mkiplookup_request_t *request);

/// mkiplookup_response_good returns true if no error occurred
/// and false otherwise. Check the logs in such case. Note that this
/// function calls abort if passsed a null @p response.
int64_t mkiplookup_response_good(const mkiplookup_response_t *response);

/// mkiplookup_response_get_bytes_sent returns the bytes sent. This
/// function calls abort if passed a null pointer.
double mkiplookup_response_get_bytes_sent(
    const mkiplookup_response_t *response);

/// mkiplookup_response_get_bytes_recv returns the bytes received. This
/// function calls abort if passed a null pointer.
double mkiplookup_response_get_bytes_recv(
    const mkiplookup_response_t *response);

/// mkiplookup_response_get_probe_ip returns the probe IP. If the lookup
/// failed, returns an empty string. Calls abort if @p response is null.
const char *mkiplookup_response_get_probe_ip(
    const mkiplookup_response_t *response);

/// mkiplookup_response_get_logs_binary returns the (possibly non UTF-8)
/// logs. Calls abort if passed any null pointer.
void mkiplookup_response_get_logs_binary(
    const mkiplookup_response_t *response,
    const uint8_t **base, size_t *count);

/// mkiplookup_response_delete destroys @p response.
void mkiplookup_response_delete(mkiplookup_response_t *response);

#ifdef __cplusplus
}  // extern "C"

#include <memory>
#include <string>

/// mkiplookup_request_deleter is a deleter for mkiplookup_request_t.
struct mkiplookup_request_deleter {
  void operator()(mkiplookup_request_t *p) {
    mkiplookup_request_delete(p);
  }
};

/// mkiplookup_request_uptr is a unique pointer to mkiplookup_request_t.
using mkiplookup_request_uptr = std::unique_ptr<
    mkiplookup_request_t, mkiplookup_request_deleter>;

/// mkiplookup_response_deleter is a deleter for mkiplookup_response_t.
struct mkiplookup_response_deleter {
  void operator()(mkiplookup_response_t *p) {
    mkiplookup_response_delete(p);
  }
};

/// mkiplookup_response_uptr is a unique pointer to mkiplookup_response_t.
using mkiplookup_response_uptr = std::unique_ptr<
    mkiplookup_response_t, mkiplookup_response_deleter>;

/// mkiplookup_response_moveout_logs moves logs out of @p response and
/// returns them. Beware that they may contain binary data. Calls abort
/// if @p response is a null pointer.
std::string mkiplookup_response_moveout_logs(
    mkiplookup_response_uptr &response);

// By default the implementation is not included. You can force it being
// included by providing the following definition to the compiler.
//
// If you're just into understanding the API, you can stop reading here.
#ifdef MKIPLOOKUP_INLINE_IMPL

#include <ctype.h>

#include "mkdata.h"
#include "mkcurl.h"

// mkiplookup_ubuntu_get_url returns the URL to perform a IPLookup using
// the GeoIP services provided by Ubuntu.
static const char *mkiplookup_ubuntu_get_url() {
  return "https://geoip.ubuntu.com/lookup";
}

// mkiplookup_ubuntu_parse parses @p body to extract a @p probe_ip. Returns
// true on success, false on failure. Aborts if @p probe_ip is null.
static int64_t
mkiplookup_ubuntu_parse(std::string &&body, std::string *probe_ip) {
  if (probe_ip == nullptr) {
    abort();
  }
  *probe_ip = "";  // reset
  {
    mkdata_uptr data{mkdata_new_nonnull()};
    mkdata_movein_data(data, std::move(body));
    if (!mkdata_contains_valid_utf8_v2(data.get())) return false;
    body = mkdata_moveout_data(data);
  }
  {
    static const std::string open_tag = "<Ip>";
    static const std::string close_tag = "</Ip>";
    auto pos = body.find(open_tag);
    if (pos == std::string::npos) return false;
    body = body.substr(pos + open_tag.size());  // Find EOS in the worst case
    pos = body.find(close_tag);
    if (pos == std::string::npos) return false;
    body = body.substr(0, pos);
    for (auto ch : body) {
      if (isspace(ch)) continue;
      ch = (char)tolower(ch);
      auto ok = isdigit(ch) || (ch >= 'a' && ch <= 'f') || ch == '.' || ch == ':';
      if (!ok) return false;
      *probe_ip += ch;
    }
  }
  return true;
}

// mkiplookup_request is a IPLookup request.
struct mkiplookup_request {
  // ca_bundle_path is the path of the CA bundle to use.
  std::string ca_bundle_path;
  // timeout is the timeout in seconds.
  int64_t timeout = 30;
};

mkiplookup_request_t *mkiplookup_request_new_nonnull() {
  return new mkiplookup_request_t;
}

void mkiplookup_request_set_timeout(
    mkiplookup_request_t *request,
    int64_t timeout) {
  if (request == nullptr) {
    abort();
  }
  request->timeout = timeout;
}

void mkiplookup_request_set_ca_bundle_path(
    mkiplookup_request_t *request,
    const char *ca_bundle_path) {
  if (request == nullptr || ca_bundle_path == nullptr) {
    abort();
  }
  request->ca_bundle_path = ca_bundle_path;
}

// mkiplookup_response is a IPLookup response.
struct mkiplookup_response {
  // good indicates whether we succeeded.
  int64_t good = false;
  // logs contains (possibly non UTF-8) logs.
  std::string logs;
  // probe_ip is the probe IP.
  std::string probe_ip;
  // bytes_sent is the bytes sent in the request.
  double bytes_sent = 0.0;
  // bytes_recv is the bytes recv in the response.
  double bytes_recv = 0.0;
};

mkiplookup_response_t *mkiplookup_request_perform_nonnull(
    const mkiplookup_request_t *request) {
  if (request == nullptr) {
    abort();
  }
  mkcurl_request_uptr r{mkcurl_request_new_nonnull()};
  mkcurl_request_set_timeout_v2(r.get(), request->timeout);
  mkcurl_request_set_ca_bundle_path_v2(
      r.get(), request->ca_bundle_path.c_str());
  mkcurl_request_set_url_v2(r.get(), mkiplookup_ubuntu_get_url());
  mkcurl_response_uptr re{mkcurl_request_perform_nonnull(r.get())};
  mkiplookup_response_uptr response{new mkiplookup_response_t};
  response->bytes_recv = mkcurl_response_get_bytes_recv_v2(re.get());
  response->bytes_sent = mkcurl_response_get_bytes_sent_v2(re.get());
  response->logs = mkcurl_response_moveout_logs_v2(re);
  if (mkcurl_response_get_error_v2(re.get()) != 0) {
    response->logs += "HTTP request failed.\n";
    return response.release();
  }
  if (mkcurl_response_get_status_code_v2(re.get()) != 200) {
    response->logs += "Status code indicate failure.\n";
    return response.release();
  }
  std::string body = mkcurl_response_moveout_body_v2(re);
  response->logs += "=== BEGIN RECEIVED BODY ===\n";
  response->logs += body;
  response->logs += "=== END RECEIVED BODY ===\n";
  if (!mkiplookup_ubuntu_parse(std::move(body), &response->probe_ip)) {
    response->logs += "Cannot parse the response body.\n";
    return response.release();
  }
  response->good = true;
  response->logs += "All good.\n";
  return response.release();
}

void mkiplookup_request_delete(mkiplookup_request_t *request) {
  delete request;
}

int64_t mkiplookup_response_good(const mkiplookup_response_t *response) {
  if (response == nullptr) {
    abort();
  }
  return response->good;
}

double mkiplookup_response_get_bytes_sent(
    const mkiplookup_response_t *response) {
  if (response == nullptr) {
    abort();
  }
  return response->bytes_sent;
}

double mkiplookup_response_get_bytes_recv(
    const mkiplookup_response_t *response) {
  if (response == nullptr) {
    abort();
  }
  return response->bytes_recv;
}

const char *mkiplookup_response_get_probe_ip(
    const mkiplookup_response_t *response) {
  if (response == nullptr) {
    abort();
  }
  return response->probe_ip.c_str();
}

void mkiplookup_response_get_logs_binary(
    const mkiplookup_response_t *response,
    const uint8_t **base, size_t *count) {
  if (response == nullptr || base == nullptr || count == nullptr) {
    abort();
  }
  *base = (const uint8_t *)response->logs.c_str();
  *count = response->logs.size();
}

void mkiplookup_response_delete(mkiplookup_response_t *response) {
  delete response;
}

std::string mkiplookup_response_moveout_logs(
    mkiplookup_response_uptr &response) {
  if (response == nullptr) {
    abort();
  }
  return std::move(response->logs);
}

#endif  // MKIPLOOKUP_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKIPLOOKUP_H
