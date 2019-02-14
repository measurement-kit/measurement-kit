// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKGEOIP_HPP
#define MEASUREMENT_KIT_MKGEOIP_HPP

/// @file mkgeoip.hpp
///
/// MkGeoIP implements OONI's IP lookup. It resolves the probe's IP, the
/// probe's ASN (autonomous system number), the probe's CC (country code),
/// and the probe's ORG (organization owning the ASN). It uses GeoLite2
/// databases in MaxMindDB format. When running on mobile, you also need
/// a CA bundle to perform TLS certificates validation.

#include <stdint.h>

#include <string>
#include <vector>

namespace mk {
namespace geoip {

/// LookupSettings contains lookup settings.
struct LookupSettings {
  /// ca_bundle_path is the CA bundle path to use.
  std::string ca_bundle_path;

  /// asn_db_path is the ASN DB path to use.
  std::string asn_db_path;

  /// country_db_path is the country DB path to use.
  std::string country_db_path;

  /// timeout is the request timeout in seconds.
  int64_t timeout = 30;
};

/// LookupResults contains lookup results.
struct LookupResults {
  /// good tells you whether we succeded.
  bool good = false;

  /// logs contains the (possibly non UTF-8) logs.
  std::vector<std::string> logs;

  /// probe_ip is the probe IP.
  std::string probe_ip;

  /// probe_asn_string is the probe ASN string. It is a string like `AS1234`.
  std::string probe_asn_string;

  /// probe_cc is the probe country code.
  std::string probe_cc;

  /// probe_org is the organization owning the ASN.
  std::string probe_org;

  /// bytes_sent is the amount of bytes sent.
  int64_t bytes_sent = 0;

  /// bytes_recv is the amount of bytes received.
  int64_t bytes_recv = 0;
};

/// lookup performs a lookup of probe_ip, probe_asn_string, probe_cc, etc.
LookupResults lookup(const LookupSettings &settings) noexcept;

}  // namespace geoip
}  // namespace mk

// By default the implementation is not included. You can force it being
// included by providing the following definition to the compiler.
//
// If you're just into understanding the API, you can stop reading here.
#ifdef MKGEOIP_INLINE_IMPL

#include <algorithm>
#include <sstream>

#include "mkiplookup.hpp"
#include "mkmmdb.hpp"

#include "mkmock.hpp"

#ifdef MKGEOIP_MOCK
#define MKGEOIP_HOOK MKMOCK_HOOK_ENABLED
#define MKGEOIP_HOOK_ALLOC MKMOCK_HOOK_ALLOC_ENABLED
#else
#define MKGEOIP_HOOK MKMOCK_HOOK_DISABLED
#define MKGEOIP_HOOK_ALLOC MKMOCK_HOOK_ALLOC_DISABLED
#endif

namespace mk {
namespace geoip {

static bool isgood(const LookupResults &results) noexcept {
  return !results.probe_ip.empty() && !results.probe_asn_string.empty()  //
         && !results.probe_cc.empty() && !results.probe_org.empty();
}

LookupResults lookup(const LookupSettings &settings) noexcept {
  LookupResults results;
  {
    iplookup::Request r;
    r.timeout = settings.timeout;
    r.ca_bundle_path = settings.ca_bundle_path;
    iplookup::Response re = iplookup::perform(r);
    results.bytes_recv = re.bytes_recv;
    results.bytes_sent = re.bytes_sent;
    std::swap(results.logs, re.logs);
    MKGEOIP_HOOK(iplookup_results_good, re.good);
    if (!re.good) {
      results.logs.push_back("IP lookup failed.");
      return results;
    }
    std::swap(results.probe_ip, re.probe_ip);
  }
  {
    mmdb::Handle db;
    bool ok = db.open(settings.country_db_path, results.logs);
    MKGEOIP_HOOK(db_open_country, ok);
    if (ok) {
      (void)db.lookup_cc(results.probe_ip, results.probe_cc, results.logs);
    } else {
      results.logs.push_back("Cannot open country database.");
    }
  }
  {
    mmdb::Handle db;
    bool ok = db.open(settings.asn_db_path, results.logs);
    MKGEOIP_HOOK(db_open_asn, ok);
    if (ok) {
      (void)db.lookup_asn2(
          results.probe_ip, results.probe_asn_string, results.logs);
      (void)db.lookup_org(results.probe_ip, results.probe_org, results.logs);
    } else {
      results.logs.push_back("Cannot open ASN database.");
    }
  }
  results.good = isgood(results);
  return results;
}

}  // namespace geoip
}  // namespace mk
#endif  // MKGEOIP_INLINE_IMPL
#endif  // MEASUREMENT_KIT_MKGEOIP_HPP
