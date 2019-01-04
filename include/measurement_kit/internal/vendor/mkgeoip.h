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

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/// mkgeoip_lookup_settings_t contains GeoIP lookup settings.
typedef struct mkgeoip_lookup_settings mkgeoip_lookup_settings_t;

/// mkgeoip_lookup_settings_new_nonnull creates a new GeoIP lookup settings
/// instance. This function aborts if allocation fails, hence it always
/// returns a valid pointer.
mkgeoip_lookup_settings_t *mkgeoip_lookup_settings_new_nonnull(void);

/// mkgeoip_lookup_settings_set_timeout_v2 sets the timeout. That is the maximum
/// time after which HTTP requests are interrupted and return error. This
/// function aborts if passed null pointers.
void mkgeoip_lookup_settings_set_timeout_v2(
    mkgeoip_lookup_settings_t *settings,
    int64_t timeout);

/// mkgeoip_lookup_settings_set_ca_bundle_path_v2 sets the CA bundle path. It
/// calls abort if passed any null pointer.
void mkgeoip_lookup_settings_set_ca_bundle_path_v2(
    mkgeoip_lookup_settings_t *settings,
    const char *ca_bundle_path);

/// mkgeoip_lookup_settings_set_country_db_path_v2 sets the country DB path. It
/// calls abort if passed any null pointer.
void mkgeoip_lookup_settings_set_country_db_path_v2(
    mkgeoip_lookup_settings_t *settings,
    const char *country_db_path);

/// mkgeoip_lookup_settings_set_asn_db_path_v2 sets the ASN DB path. It
/// calls abort if passed any null pointer.
void mkgeoip_lookup_settings_set_asn_db_path_v2(
    mkgeoip_lookup_settings_t *settings,
    const char *asn_db_path);

/// mkgeoip_lookup_results_t contains GeoIP lookup results.
typedef struct mkgeoip_lookup_results mkgeoip_lookup_results_t;

/// mkgeoip_lookup_settings_perform_nonnull performs a lookup with @p
/// settings. Always returns a valid pointer. Calls abort if it is
/// passed a null pointer @p settings.
mkgeoip_lookup_results_t *mkgeoip_lookup_settings_perform_nonnull(
    const mkgeoip_lookup_settings_t *settings);

/// mkgeoip_lookup_settings_delete destroys @p settings. Note that
/// @p settings MAY be null.
void mkgeoip_lookup_settings_delete(mkgeoip_lookup_settings_t *settings);

/// mkgeoip_lookup_results_good_v2 returns true if no error occurred during
/// the GeoIP lookup, and false otherwise. Check the logs in such case. It
/// calls abort if passed a null @p results.
int64_t mkgeoip_lookup_results_good_v2(const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_bytes_sent_v2 returns the bytes sent. It
/// calls abort if passed a null pointer.
double mkgeoip_lookup_results_get_bytes_sent_v2(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_bytes_recv_v2 returns the bytes received. It
/// calls abort if passed a null pointer.
double mkgeoip_lookup_results_get_bytes_recv_v2(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_probe_ip_v2 returns the probe IP. If the lookup
/// failed, returns an empty string. Calls abort if @p results is null.
const char *mkgeoip_lookup_results_get_probe_ip_v2(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_probe_asn_v2 returns the probe ASN. If the lookup
/// failed, returns the zero ASN, which is reserved. Calls abort if it's
/// passed a null @p results.
int64_t mkgeoip_lookup_results_get_probe_asn_v2(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_probe_cc_v2 returns the probe CC. If the lookup
/// failed, returns an empty string. Aborts if @p results is null.
const char *mkgeoip_lookup_results_get_probe_cc_v2(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_probe_org_v2 returns the probe ORG. If the lookup
/// failed, returns an empty string. Aborts if @p results is a null pointer.
const char *mkgeoip_lookup_results_get_probe_org_v2(
    const mkgeoip_lookup_results_t *results);

/// mkgeoip_lookup_results_get_logs_binary_v2 returns the (possibly binary) logs
/// of the IP lookup. Aborts if passed any null pointer.
void mkgeoip_lookup_results_get_logs_binary_v2(
    const mkgeoip_lookup_results_t *results,
    const uint8_t **base, size_t *count);

/// mkgeoip_lookup_results_delete destroys @p results, which MAY be null.
void mkgeoip_lookup_results_delete(mkgeoip_lookup_results_t *results);

#ifdef __cplusplus
}  // extern "C"

#include <memory>
#include <string>

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

/// mkgeoip_lookup_results_moveout_logs_v2 moves logs out of @p results. It
/// will call abort if passed a null pointer.
std::string mkgeoip_lookup_results_moveout_logs_v2(
    mkgeoip_lookup_results_t *results);

// By default the implementation is not included. You can force it being
// included by providing the following definition to the compiler.
//
// If you're just into understanding the API, you can stop reading here.
#ifdef MKGEOIP_INLINE_IMPL

#include <sstream>
#include <vector>

#include "mkiplookup.h"
#include "mkmmdb.h"

// mkgeoip_lookup_settings contains GeoIP lookup settings.
struct mkgeoip_lookup_settings {
  // ca_bundle_path is the CA bundle path to use.
  std::string ca_bundle_path;
  // asn_db_path is the ASN DB path to use.
  std::string asn_db_path;
  // country_db_path is the country DB path to use.
  std::string country_db_path;
  // timeout is the request timeout in seconds.
  int64_t timeout = 30;
};

mkgeoip_lookup_settings_t *mkgeoip_lookup_settings_new_nonnull() {
  return new mkgeoip_lookup_settings_t;
}

void mkgeoip_lookup_settings_set_timeout_v2(
    mkgeoip_lookup_settings_t *settings,
    int64_t timeout) {
  if (settings == nullptr) {
    abort();
  }
  settings->timeout = timeout;
}

void mkgeoip_lookup_settings_set_ca_bundle_path_v2(
    mkgeoip_lookup_settings_t *settings,
    const char *ca_bundle_path) {
  if (settings == nullptr || ca_bundle_path == nullptr) {
    abort();
  }
  settings->ca_bundle_path = ca_bundle_path;
}

void mkgeoip_lookup_settings_set_country_db_path_v2(
    mkgeoip_lookup_settings_t *settings,
    const char *country_db_path) {
  if (settings == nullptr || country_db_path == nullptr) {
    abort();
  }
  settings->country_db_path = country_db_path;
}

void mkgeoip_lookup_settings_set_asn_db_path_v2(
    mkgeoip_lookup_settings_t *settings,
    const char *asn_db_path) {
  if (settings == nullptr || asn_db_path == nullptr) {
    abort();
  }
  settings->asn_db_path = asn_db_path;
}

// mkgeoip_lookup_results contains GeoIP lookup results.
struct mkgeoip_lookup_results {
  // good tells you whether we succeded.
  int64_t good = false;
  // logs contains the (possibly non UTF-8) logs.
  std::string logs;
  // probe_ip is the probe IP.
  std::string probe_ip;
  // probe_asn is the probe ASN.
  int64_t probe_asn = 0;
  // probe_cc is the probe country code.
  std::string probe_cc;
  // probe_org is the organization owning the ASN.
  std::string probe_org;
  // bytes_sent is the amount of bytes sent.
  double bytes_sent = 0.0;
  // bytes_recv is the amount of bytes received.
  double bytes_recv = 0.0;
};

mkgeoip_lookup_results_t *mkgeoip_lookup_settings_perform_nonnull(
    const mkgeoip_lookup_settings_t *settings) {
  if (settings == nullptr) {
    abort();
  }
  mkgeoip_lookup_results_uptr results{new mkgeoip_lookup_results_t};
  {
    mkiplookup_request_uptr r{mkiplookup_request_new_nonnull()};
    mkiplookup_request_set_timeout(r.get(), settings->timeout);
    mkiplookup_request_set_ca_bundle_path(
        r.get(), settings->ca_bundle_path.c_str());
    mkiplookup_response_uptr re{mkiplookup_request_perform_nonnull(r.get())};
    results->bytes_recv = mkiplookup_response_get_bytes_recv(re.get());
    results->bytes_sent = mkiplookup_response_get_bytes_sent(re.get());
    results->logs = mkiplookup_response_moveout_logs(re);
    if (!mkiplookup_response_good(re.get())) {
      results->logs += "IPLookup failed.\n";
      return results.release();
    }
    results->probe_ip = mkiplookup_response_get_probe_ip(re.get());
  }
  {
    mkmmdb_uptr db{mkmmdb_open_nonnull(settings->country_db_path.c_str())};
    if (mkmmdb_good(db.get())) {
      results->probe_cc = mkmmdb_lookup_cc(db.get(), results->probe_ip.c_str());
      results->logs += mkmmdb_get_last_lookup_logs(db.get());
    } else {
      results->logs += "Cannot open country database.\n";
    }
  }
  {
    mkmmdb_uptr db{mkmmdb_open_nonnull(settings->asn_db_path.c_str())};
    if (mkmmdb_good(db.get())) {
      results->probe_asn = mkmmdb_lookup_asn(
          db.get(), results->probe_ip.c_str());
      results->logs += mkmmdb_get_last_lookup_logs(db.get());
      results->probe_org = mkmmdb_lookup_org(
          db.get(), results->probe_ip.c_str());
      results->logs += mkmmdb_get_last_lookup_logs(db.get());
    } else {
      results->logs += "Cannot open ASN database.\n";
    }
  }
  results->good = !results->probe_ip.empty() && results->probe_asn != 0  //
                  && !results->probe_cc.empty() && !results->probe_org.empty();
  results->logs += "All good.\n";
  return results.release();
}

void mkgeoip_lookup_settings_delete(mkgeoip_lookup_settings_t *settings) {
  delete settings;
}

int64_t mkgeoip_lookup_results_good_v2(
    const mkgeoip_lookup_results_t *results) {
  if (results == nullptr) {
    abort();
  }
  return results->good;
}

double mkgeoip_lookup_results_get_bytes_sent_v2(
    const mkgeoip_lookup_results_t *results) {
  if (results == nullptr) {
    abort();
  }
  return results->bytes_sent;
}

double mkgeoip_lookup_results_get_bytes_recv_v2(
    const mkgeoip_lookup_results_t *results) {
  if (results == nullptr) {
    abort();
  }
  return results->bytes_recv;
}

const char *mkgeoip_lookup_results_get_probe_ip_v2(
    const mkgeoip_lookup_results_t *results) {
  if (results == nullptr) {
    abort();
  }
  return results->probe_ip.c_str();
}

int64_t mkgeoip_lookup_results_get_probe_asn_v2(
    const mkgeoip_lookup_results_t *results) {
  if (results == nullptr) {
    abort();
  }
  return results->probe_asn;
}

const char *mkgeoip_lookup_results_get_probe_cc_v2(
    const mkgeoip_lookup_results_t *results) {
  if (results == nullptr) {
    abort();
  }
  return results->probe_cc.c_str();
}

const char *mkgeoip_lookup_results_get_probe_org_v2(
    const mkgeoip_lookup_results_t *results) {
  if (results == nullptr) {
    abort();
  }
  return results->probe_org.c_str();
}

void mkgeoip_lookup_results_get_logs_binary_v2(
    const mkgeoip_lookup_results_t *results,
    const uint8_t **base, size_t *count) {
  if (results == nullptr || base == nullptr || count == nullptr) {
    abort();
  }
  *base = (const uint8_t *)results->logs.c_str();
  *count = results->logs.size();
}

void mkgeoip_lookup_results_delete(mkgeoip_lookup_results_t *results) {
  delete results;
}

std::string mkgeoip_lookup_results_moveout_logs_v2(
    mkgeoip_lookup_results_t *results) {
  if (results == nullptr) {
    abort();
  }
  return std::move(results->logs);
}

#endif  // MKGEOIP_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKGEOIP_H
