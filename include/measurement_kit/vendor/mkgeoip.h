// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKGEOIP_H
#define MEASUREMENT_KIT_MKGEOIP_H

/// @file mkgeoip.h
///
/// MkGeoIP implements OONI's IP lookup. It resolves the probe's IP, the
/// probe's ASN (autonomous system number), the probe's CC (country code),
/// and the probe's ORG (organization owning the ASN). It uses GeoLite2
/// databases in MaxMindDB format. When running on mobile, you also need
/// a CA bundle to perform TLS certificates validation.
///
/// This file implements several low-level and high-level APIs. The high
/// level API consists of configurable lookup settings
/// (mkgeoip_lookup_settings_t) and results (mkgeoip_lookup_results_t).

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/// mkgeoip_ubuntu_request_get_url returns the URL for querying the
/// geolocation service provided by Ubuntu. This function MAY return NULL
/// in case of serious internal error.
const char *mkgeoip_ubuntu_request_get_url(void);

/// mkgeoip_ubuntu_request_get_method returns the method for querying the
/// geolocation service provided by Ubuntu. This function MAY return a NULL
/// pointer in case of serious internal error.
const char *mkgeoip_ubuntu_request_get_method(void);

/// mkgeoip_ubuntu_response_t is a response returned by the
/// geolocation service provided by Ubuntu.
typedef struct mkgeoip_ubuntu_response mkgeoip_ubuntu_response_t;

/// mkgeoip_ubuntu_response_new creates a new ubuntu geolocation response.
mkgeoip_ubuntu_response_t *mkgeoip_ubuntu_response_new(void);

/// mkgeoip_ubuntu_response_set_status_code sets the status code received
/// by the geolocation service provided by Ubuntu.
void mkgeoip_ubuntu_response_set_status_code(
    mkgeoip_ubuntu_response_t *response,
    int64_t status_code);

/// mkgeoip_ubuntu_response_set_content_type sets the content type received
/// by the geolocation service provided by Ubuntu.
void mkgeoip_ubuntu_response_set_content_type(
    mkgeoip_ubuntu_response_t *response,
    const char *content_type);

/// mkgeoip_ubuntu_response_set_body_binary sets the binary response body
/// received by the geolocation service provided by Ubuntu.
void mkgeoip_ubuntu_response_set_body_binary(
    mkgeoip_ubuntu_response_t *response,
    const uint8_t *base, size_t count);

/// mkgeoip_ubuntu_response_parse parses the fields previously set inside
/// of @p response to extract the IP address. Returns true on success, and
/// false on failure. On failure, inspect the original HTTP response body
/// and status code to understand what went wrong.
int64_t mkgeoip_ubuntu_response_parse(mkgeoip_ubuntu_response_t *response);

/// mkgeoip_ubuntu_response_get_probe_ip returns the probe IP previously
/// parsed using mkgeoip_ubuntu_response_parse. If parsing did not succeed
/// it returns an empty string. MAY return NULL on internal error. The
/// returned string is valid as long as @p response is alive and no other
/// API is invoked on the @p response instance.
const char *mkgeoip_ubuntu_response_get_probe_ip(
    mkgeoip_ubuntu_response_t *response);

/// mkgeoip_ubuntu_response_delete destroys @p response.
void mkgeoip_ubuntu_response_delete(mkgeoip_ubuntu_response_t *response);

/// mkgeoip_mmdb_t is an open MMDB database.
typedef struct mkgeoip_mmdb mkgeoip_mmdb_t;

/// mkgeoip_mmdb_open opens the database at @p path and returns the
/// database instance on success, or NULL on failure.
mkgeoip_mmdb_t *mkgeoip_mmdb_open(const char *path);

/// mkgeoip_mmdb_lookup_cc returns the country code of @p ip using the
/// @p mmdb database, or NULL in case of error. The returned string will
/// be valid until @p mmdb is valid _and_ you don't call other lookup
/// APIs using the same @p mmdb instance.
const char *mkgeoip_mmdb_lookup_cc(mkgeoip_mmdb_t *mmdb, const char *ip);

/// mkgeoip_mmdb_lookup_asn is like mkgeoip_mmdb_lookup_cc but returns
/// the ASN on success and zero on failure.
int64_t mkgeoip_mmdb_lookup_asn(mkgeoip_mmdb_t *mmdb, const char *ip);

/// mkgeoip_mmdb_lookup_org is like mkgeoip_mmdb_lookup_cc but returns
/// the organization bound to @p ip on success, NULL on failure. The
/// returned string will be valid until @p mmdb is valid _and_ you don't
/// call other lookup APIs using the same @p mmdb instance.
const char *mkgeoip_mmdb_lookup_org(mkgeoip_mmdb_t *mmdb, const char *ip);

/// mkgeoip_mmdb_close closes @p mmdb.
void mkgeoip_mmdb_close(mkgeoip_mmdb_t *mmdb);

/// mkgeoip_lookup_settings_t contains GeoIP lookup settings.
typedef struct mkgeoip_lookup_settings mkgeoip_lookup_settings_t;

/// mkgeoip_lookup_settings_new creates a new GeoIP lookup settings instance.
mkgeoip_lookup_settings_t *mkgeoip_lookup_settings_new(void);

void mkgeoip_lookup_settings_set_timeout(
    mkgeoip_lookup_settings_t *settings,
    int64_t timeout);

/// mkgeoip_lookup_settings_set_ca_bundle_path sets the CA bundle path.
void mkgeoip_lookup_settings_set_ca_bundle_path(
    mkgeoip_lookup_settings_t *settings,
    const char *ca_bundle_path);

/// mkgeoip_lookup_settings_set_country_db_path sets the country DB path.
void mkgeoip_lookup_settings_set_country_db_path(
    mkgeoip_lookup_settings_t *settings,
    const char *country_db_path);

/// mkgeoip_lookup_settings_set_asn_db_path sets the ASN DB path.
void mkgeoip_lookup_settings_set_asn_db_path(
    mkgeoip_lookup_settings_t *settings,
    const char *asn_db_path);

/// mkgeoip_lookup_results_t contains GeoIP lookup results.
typedef struct mkgeoip_lookup_results mkgeoip_lookup_results_t;

/// mkgeoip_lookup_settings_perform performs a lookup with @p settings. MAY
/// return NULL in case of serious internal errors.
mkgeoip_lookup_results_t *mkgeoip_lookup_settings_perform(
    const mkgeoip_lookup_settings_t *settings);

/// mkgeoip_lookup_settings_delete destroys @p settings.
void mkgeoip_lookup_settings_delete(mkgeoip_lookup_settings_t *settings);

/// mkgeoip_lookup_results_good returns true if no error occurred during
/// the GeoIP lookup, and false otherwise. Check the logs in such case.
int64_t mkgeoip_lookup_results_good(const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_bytes_sent returns the bytes sent.
double mkgeoip_lookup_results_get_bytes_sent(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_bytes_recv returns the bytes received.
double mkgeoip_lookup_results_get_bytes_recv(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_probe_ip returns the probe IP. If the lookup
/// failed, returns an empty string. MAY return NULL on internal error.
const char *mkgeoip_lookup_results_get_probe_ip(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_probe_asn returns the probe ASN. If the lookup
/// failed, returns the zero ASN, which is reseved.
int64_t mkgeoip_lookup_results_get_probe_asn(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_probe_cc returns the probe CC. If the lookup
/// failed, returns an empty string. MAY return NULL on internal error.
const char *mkgeoip_lookup_results_get_probe_cc(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_probe_org returns the probe ORG. If the lookup
/// failed, returns an empty string. MAY return NULL on internal error.
const char *mkgeoip_lookup_results_get_probe_org(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_logs_binary returns the (possibly binary) logs
/// of the IP lookup. Returns true on success, false on failure.
int64_t mkgeoip_lookup_results_get_logs_binary(
    const mkgeoip_lookup_results_t *results,
    const uint8_t **base, size_t *count);

/// mkgeoip_lookup_results_delete destroys @p results.
void mkgeoip_lookup_results_delete(mkgeoip_lookup_results_t *results);

#ifdef __cplusplus
}  // extern "C"

#include <memory>
#include <string>

/// mkgeoip_ubuntu_response_deleter is a deleter for mkgeoip_ubuntu_response_t.
struct mkgeoip_ubuntu_response_deleter {
  void operator()(mkgeoip_ubuntu_response_t *p) {
    mkgeoip_ubuntu_response_delete(p);
  }
};

/// mkgeoip_ubuntu_response_uptr is a unique pointer to mkgeoip_ubuntu_response_t.
using mkgeoip_ubuntu_response_uptr = std::unique_ptr<
    mkgeoip_ubuntu_response_t, mkgeoip_ubuntu_response_deleter>;

/// mkgeoip_mmdb_deleter is a deleter for mkgeoip_mmdb_t.
struct mkgeoip_mmdb_deleter {
  void operator()(mkgeoip_mmdb_t *p) { mkgeoip_mmdb_close(p); }
};

/// mkgeoip_mmdb_uptr is a unique pointer to mkgeoip_mmdb_t.
using mkgeoip_mmdb_uptr = std::unique_ptr<mkgeoip_mmdb_t, mkgeoip_mmdb_deleter>;

/// mkgeoip_ubuntu_response_movein_body moves @p body as the @p response body
/// and returns true on success and false on failure.
int64_t mkgeoip_ubuntu_response_movein_body(
    mkgeoip_ubuntu_response_t *response,
    std::string &&body);

/// mkgeoip_lookup_settings_deleter is a deleter for mkgeoip_lookup_settings_t.
struct mkgeoip_lookup_settings_deleter {
  void operator()(mkgeoip_lookup_settings_t *p) {
    mkgeoip_lookup_settings_delete(p);
  }
};

/// mkgeoip_lookup_settings_uptr is a unique pointer to
/// mkgeoip_lookup_settings_t.
using mkgeoip_lookup_settings_uptr = std::unique_ptr<
    mkgeoip_lookup_settings_t, mkgeoip_lookup_settings_deleter>;

/// mkgeoip_lookup_results_deleter is a deleter for mkgeoip_lookup_results_t.
struct mkgeoip_lookup_results_deleter {
  void operator()(mkgeoip_lookup_results_t *p) {
    mkgeoip_lookup_results_delete(p);
  }
};

/// mkgeoip_lookup_results_uptr is a unique pointer to
/// mkgeoip_lookup_results_t.
using mkgeoip_lookup_results_uptr = std::unique_ptr<
    mkgeoip_lookup_results_t, mkgeoip_lookup_results_deleter>;

/// mkgeoip_lookup_results_moveout_logs moves logs from @p results into @p logs.
int64_t mkgeoip_lookup_results_moveout_logs(
    mkgeoip_lookup_results_t *results, std::string *logs);

// By default the implementation is not included. You can force it being
// included by providing the following definition to the compiler.
//
// If you're just into understanding the API, you can stop reading here.
#ifdef MKGEOIP_INLINE_IMPL

#include <ctype.h>

#include <chrono>
#include <functional>
#include <sstream>
#include <vector>

#include <maxminddb.h>

#include "mkdata.h"
#include "mkcurl.h"

const char *mkgeoip_ubuntu_request_get_url() {
  return "https://geoip.ubuntu.com/lookup";
}

const char *mkgeoip_ubuntu_request_get_method() {
  return "GET";
}

struct mkgeoip_ubuntu_response {
  std::string content_type;
  std::string body;
  int64_t status_code = 0;
  std::string probe_ip;
};

mkgeoip_ubuntu_response_t *mkgeoip_ubuntu_response_new() {
  return new mkgeoip_ubuntu_response_t;
}

void mkgeoip_ubuntu_response_set_status_code(
    mkgeoip_ubuntu_response_t *response,
    int64_t status_code) {
  if (response != nullptr) response->status_code = status_code;
}

void mkgeoip_ubuntu_response_set_content_type(
    mkgeoip_ubuntu_response_t *response,
    const char *content_type) {
  if (response != nullptr && content_type != nullptr) {
    response->content_type = content_type;
  }
}

void mkgeoip_ubuntu_response_set_body_binary(
    mkgeoip_ubuntu_response_t *response,
    const uint8_t *base, size_t count) {
  if (response != nullptr && base != nullptr && count > 0) {
    response->body = std::string{(const char *)base, count};
  }
}

int64_t mkgeoip_ubuntu_response_parse(mkgeoip_ubuntu_response_t *response) {
  if (response == nullptr) return false;
  response->probe_ip = "";  // reset
  if (response->status_code != 200) return false;
  if (response->content_type != "text/xml" &&
      response->content_type != "application/xml") {
    // NOTHING - TODO(bassosimone): make this code more robust by taking
    // into account the "text/xml; encoding=utf-8" possibility. Until that
    // point, returning error in this case is too strict.
  }
  std::string body = response->body;
  {
    mkdata_uptr data{mkdata_new()};
    mkdata_movein(data.get(), std::move(body));
    if (!mkdata_contains_valid_utf8(data.get())) return false;
    body = mkdata_moveout(data.get());
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
      response->probe_ip += ch;
    }
  }
  return true;
}

const char *mkgeoip_ubuntu_response_get_probe_ip(
    mkgeoip_ubuntu_response_t *response) {
  return (response != nullptr) ? response->probe_ip.c_str() : nullptr;
}

void mkgeoip_ubuntu_response_delete(mkgeoip_ubuntu_response_t *response) {
  delete response;
}

struct mkgeoip_mmdb_s_deleter {
  void operator()(MMDB_s *p) {
    MMDB_close(p);
    delete p;
  }
};

using mkgeoip_mmdb_s_uptr = std::unique_ptr<MMDB_s, mkgeoip_mmdb_s_deleter>;

struct mkgeoip_mmdb {
  mkgeoip_mmdb_s_uptr mmdbs;
  std::string saved_string;
};

#ifndef MKGEOIP_MMDB_OPEN
// MKGEOIP_MMDB_OPEN allows to mock MMDB_open
#define MKGEOIP_MMDB_OPEN MMDB_open
#endif

mkgeoip_mmdb_t *mkgeoip_mmdb_open(const char *path) {
  if (path == nullptr) return nullptr;
  mkgeoip_mmdb_uptr mmdb{new mkgeoip_mmdb_t};
  mmdb->mmdbs.reset(new MMDB_s);
  if (MKGEOIP_MMDB_OPEN(path, MMDB_MODE_MMAP, mmdb->mmdbs.get()) != 0) {
    return nullptr;
  }
  return mmdb.release();
}

#ifndef MKGEOIP_MMDB_GET_VALUE
// MKGEOIP_MMDB_GET_VALUE allows to mock MMDB_get_value
#define MKGEOIP_MMDB_GET_VALUE MMDB_get_value
#endif

#ifndef MKGEOIP_MMDB_LOOKUP_STRING
// MKGEOIP_MMDB_LOOKUP_STRING allows to mock MMDB_lookup_string
#define MKGEOIP_MMDB_LOOKUP_STRING MMDB_lookup_string
#endif

static void mkgeoip_lookup_mmdb(
    MMDB_s *mmdbp, const std::string &ip,
    std::function<void(MMDB_entry_s *)> fun) {
  auto gai_error = 0;
  auto mmdb_error = 0;
  auto record = MKGEOIP_MMDB_LOOKUP_STRING(mmdbp, ip.c_str(), &gai_error,
                                           &mmdb_error);
  if (gai_error == 0 && mmdb_error == 0 && record.found_entry == true) {
    fun(&record.entry);
  }
}

const char *mkgeoip_mmdb_lookup_cc(mkgeoip_mmdb_t *mmdb, const char *ip) {
  if (mmdb == nullptr || ip == nullptr) {
    return nullptr;
  }
  const char *rv = nullptr;
  mkgeoip_lookup_mmdb(
      mmdb->mmdbs.get(), ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
            entry, &data, "registered_country", "iso_code", nullptr);
        if (mmdb_error != 0) return;
        if (!data.has_data || data.type != MMDB_DATA_TYPE_UTF8_STRING) {
          return;
        }
        mmdb->saved_string = std::string{data.utf8_string, data.data_size};
        rv = mmdb->saved_string.c_str();
      });
  return rv;
}

int64_t mkgeoip_mmdb_lookup_asn(mkgeoip_mmdb_t *mmdb, const char *ip) {
  if (mmdb == nullptr || ip == nullptr) {
    return 0;
  }
  int64_t rv;
  mkgeoip_lookup_mmdb(
      mmdb->mmdbs.get(), ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
            entry, &data, "autonomous_system_number", nullptr);
        if (mmdb_error != 0) return;
        if (!data.has_data || data.type != MMDB_DATA_TYPE_UINT32) {
          return;
        }
        rv = data.uint32;
      });
  return rv;
}

const char *mkgeoip_mmdb_lookup_org(mkgeoip_mmdb_t *mmdb, const char *ip) {
  if (mmdb == nullptr || ip == nullptr) {
    return nullptr;
  }
  const char *rv = nullptr;
  mkgeoip_lookup_mmdb(
      mmdb->mmdbs.get(), ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
            entry, &data, "autonomous_system_organization", nullptr);
        if (mmdb_error != 0) return;
        if (!data.has_data || data.type != MMDB_DATA_TYPE_UTF8_STRING) {
          return;
        }
        mmdb->saved_string = std::string{data.utf8_string, data.data_size};
        rv = mmdb->saved_string.c_str();
      });
  return rv;
}

void mkgeoip_mmdb_close(mkgeoip_mmdb_t *mmdb) { delete mmdb; }

struct mkgeoip_lookup_settings {
  std::string ca_bundle_path;
  std::string asn_db_path;
  std::string country_db_path;
  int64_t timeout = 0;
};

mkgeoip_lookup_settings_t *mkgeoip_lookup_settings_new() {
  return new mkgeoip_lookup_settings_t;
}

void mkgeoip_lookup_settings_set_timeout(
    mkgeoip_lookup_settings_t *settings,
    int64_t timeout) {
  if (settings != nullptr) settings->timeout = timeout;
}

void mkgeoip_lookup_settings_set_ca_bundle_path(
    mkgeoip_lookup_settings_t *settings,
    const char *ca_bundle_path) {
  if (settings != nullptr && ca_bundle_path != nullptr) {
    settings->ca_bundle_path = ca_bundle_path;
  }
}

void mkgeoip_lookup_settings_set_country_db_path(
    mkgeoip_lookup_settings_t *settings,
    const char *country_db_path) {
  if (settings != nullptr && country_db_path != nullptr) {
    settings->country_db_path = country_db_path;
  }
}

void mkgeoip_lookup_settings_set_asn_db_path(
    mkgeoip_lookup_settings_t *settings,
    const char *asn_db_path) {
  if (settings != nullptr && asn_db_path != nullptr) {
    settings->asn_db_path = asn_db_path;
  }
}

struct mkgeoip_lookup_results {
  int64_t good = false;
  std::string logs;
  std::string probe_ip;
  int64_t probe_asn = 0;
  std::string probe_cc;
  std::string probe_org;
  double bytes_sent = 0.0;
  double bytes_recv = 0.0;
};

// mkgeoip_results_log appends @p line to @p logs. It adds information on the
// current time in millisecond. It also appends a newline to the EOL.
static void mkgeoip_results_log(std::string *logs, std::string &&line) {
  if (logs == nullptr) abort();
  std::stringstream ss;
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch());
  ss << "[" << now.count() << "] " << line << "\n";
  *logs += ss.str();
}

mkgeoip_lookup_results_t *mkgeoip_lookup_settings_perform(
    const mkgeoip_lookup_settings_t *settings) {
  if (settings == nullptr) return nullptr;
  mkcurl_request_uptr request{mkcurl_request_new()};
  if (request == nullptr) return nullptr;
  if (settings->timeout > 0) {
    mkcurl_request_set_timeout(request.get(), settings->timeout);
  }
  mkcurl_request_set_ca_bundle_path(
      request.get(), settings->ca_bundle_path.c_str());
  mkcurl_request_set_url(request.get(), mkgeoip_ubuntu_request_get_url());
  mkcurl_response_uptr response{mkcurl_request_perform(request.get())};
  if (response == nullptr) return nullptr;
  mkgeoip_lookup_results_uptr results{new mkgeoip_lookup_results_t};
  if (results == nullptr) return nullptr;  // should not happen
  results->bytes_recv = mkcurl_response_get_bytes_recv(response.get());
  results->bytes_sent = mkcurl_response_get_bytes_sent(response.get());
  if (!mkcurl_response_moveout_logs(response.get(), &results->logs)) {
    return nullptr;
  }
  mkgeoip_ubuntu_response_uptr ubuntu{mkgeoip_ubuntu_response_new()};
  if (!ubuntu) return nullptr;
  mkgeoip_results_log(
      &results->logs, "Got response from iplookup service; parsing response");
  mkgeoip_ubuntu_response_set_status_code(
      ubuntu.get(), mkcurl_response_get_status_code(response.get()));
  mkgeoip_ubuntu_response_set_content_type(
      ubuntu.get(), mkcurl_response_get_content_type(response.get()));
  {
    std::string body;
    if (!mkcurl_response_moveout_body(response.get(), &body) ||
        !mkgeoip_ubuntu_response_movein_body(ubuntu.get(), std::move(body))) {
      mkgeoip_results_log(&results->logs, "Cannot move response body");
      return results.release();
    }
  }
  if (!mkgeoip_ubuntu_response_parse(ubuntu.get())) {
    mkgeoip_results_log(&results->logs, "Cannot parse XML returned by Ubuntu");
    return results.release();
  }
  {
    const char *s = mkgeoip_ubuntu_response_get_probe_ip(ubuntu.get());
    if (s == nullptr) {
      mkgeoip_results_log(&results->logs, "The probe IP is null");
      return results.release();
    }
    results->probe_ip = s;
  }
  mkgeoip_results_log(&results->logs, "Parsed IP; now using GeoLite2 DBs");
  {
    mkgeoip_mmdb_uptr db{mkgeoip_mmdb_open(
        settings->country_db_path.c_str())};
    if (db == nullptr) {
      mkgeoip_results_log(&results->logs, "Cannot open country file");
      return results.release();
    }
    const char *s = mkgeoip_mmdb_lookup_cc(db.get(), results->probe_ip.c_str());
    if (s == nullptr) {
      mkgeoip_results_log(&results->logs, "Cannot lookup probe CC");
    } else {
      results->probe_cc = s;
    }
  }
  {
    mkgeoip_mmdb_uptr db{mkgeoip_mmdb_open(
        settings->asn_db_path.c_str())};
    if (db == nullptr) {
      mkgeoip_results_log(&results->logs, "Cannot open ASN file");
      return results.release();
    }
    results->probe_asn = mkgeoip_mmdb_lookup_asn(
        db.get(), results->probe_ip.c_str());
    if (results->probe_asn == 0) {
      mkgeoip_results_log(&results->logs, "Cannot lookup probe ASN");
    }
    const char *s = mkgeoip_mmdb_lookup_org(
        db.get(), results->probe_ip.c_str());
    if (s == nullptr) {
      mkgeoip_results_log(&results->logs, "Cannot lookup probe ORG");
    } else {
      results->probe_org = s;
    }
  }
  results->good = !results->probe_ip.empty() && results->probe_asn != 0  //
                  && !results->probe_cc.empty() && !results->probe_org.empty();
  mkgeoip_results_log(&results->logs, "GeoIP lookup now complete");
  return results.release();
}

void mkgeoip_lookup_settings_delete(mkgeoip_lookup_settings_t *settings) {
  delete settings;
}

int64_t mkgeoip_lookup_results_good(const mkgeoip_lookup_results_t *results) {
  return (results != nullptr) ? results->good : false;
}

double mkgeoip_lookup_results_get_bytes_sent(
    const mkgeoip_lookup_results_t *results) {
  return (results != nullptr) ? results->bytes_sent : 0.0;
}

double mkgeoip_lookup_results_get_bytes_recv(
    const mkgeoip_lookup_results_t *results) {
  return (results != nullptr) ? results->bytes_recv : 0.0;
}

const char *mkgeoip_lookup_results_get_probe_ip(
    const mkgeoip_lookup_results_t *results) {
  return (results != nullptr) ? results->probe_ip.c_str() : nullptr;
}

int64_t mkgeoip_lookup_results_get_probe_asn(
    const mkgeoip_lookup_results_t *results) {
  return (results != nullptr) ? results->probe_asn : 0;
}

const char *mkgeoip_lookup_results_get_probe_cc(
    const mkgeoip_lookup_results_t *results) {
  return (results != nullptr) ? results->probe_cc.c_str() : nullptr;
}

const char *mkgeoip_lookup_results_get_probe_org(
    const mkgeoip_lookup_results_t *results) {
  return (results != nullptr) ? results->probe_org.c_str() : nullptr;
}

int64_t mkgeoip_lookup_results_get_logs_binary(
    const mkgeoip_lookup_results_t *results,
    const uint8_t **base, size_t *count) {
  if (results == nullptr || base == nullptr || count == nullptr) return false;
  *base = (const uint8_t *)results->logs.c_str();
  *count = results->logs.size();
  return true;
}

void mkgeoip_lookup_results_delete(mkgeoip_lookup_results_t *results) {
  delete results;
}

int64_t mkgeoip_ubuntu_response_movein_body(
    mkgeoip_ubuntu_response_t *response,
    std::string &&body) {
  if (response == nullptr) return false;
  std::swap(response->body, body);
  return true;
}

int64_t mkgeoip_lookup_results_moveout_logs(
    mkgeoip_lookup_results_t *results, std::string *logs) {
  if (logs == nullptr || results == nullptr) return false;
  std::swap(results->logs, *logs);
  return true;
}

#endif  // MKGEOIP_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKGEOIP_H
