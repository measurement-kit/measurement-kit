#ifndef MEASUREMENT_KIT_LIBMMDBX_LIBMMDBX_H
#define MEASUREMENT_KIT_LIBMMDBX_LIBMMDBX_H
#ifdef __cplusplus
#include <memory>  // for std::unique_ptr<T>
extern "C" {
#endif

// Libmmdbx discovers the probe IP, country code (CC), autonomous system
// numer (ASN), and network name (i.e. the ASN description).
//
// This is the typical workflow of using libmmdbx:
//
// 1. create a mk_mmdbx_settings_t instance
//
// 2. populate the settings
//
// 3. call mk_mmdbx_lookup() with the specified settings to obtain
//    a mk_mmdbx_results_t instance
//
// 4. destroy the settings instance
//
// 5. use the results (make sure you check whether an error occurred
//    by using mk_mmdbx_results_get_error() first)
//
// 6. destroy the results instance

// mk_mmdbx_error enumerates all possible error conditions.
typedef enum mk_mmdbx_error {
  MK_MMDBX_ENONE,
  MK_MMDBX_EFAULT,
  MK_MMDBX_ECURL,
  MK_MMDBX_ENOMEM,
  MK_MMDBX_EHTTP,
  MK_MMDBX_EMMDBGETVALUE,
  MK_MMDBX_ENODATAFORTYPE,
  MK_MMDBX_ENOTFOUND,
  MK_MMDBX_EGETADDRINFO,
  MK_MMDBX_EMMDBLOOKUPSTRING,
  MK_MMDBX_EMMDBOPEN,
} mk_mmdbx_error_t;

// mk_mmdbx_settings_t contains the settings.
typedef struct mk_mmdbx_settings mk_mmdbx_settings_t;

// mk_mmdbx_settings_new creates a new settings instance.
mk_mmdbx_settings_t *mk_mmdbx_settings_new(void);

// mk_mmdbx_settings_set_timeout changes the default timeout for I/O.
void mk_mmdbx_settings_set_timeout(mk_mmdbx_settings_t *p, unsigned v);

// mk_mmdbx_settings_set_country_db_path sets the path to the county
// database. We currently use libmaxminddb as a backend.
void mk_mmdbx_settings_set_country_db_path(mk_mmdbx_settings_t *p,
                                           const char *v);

// mk_mmdbx_settings_set_asn_db_path sets the path to the ASN
// database. We currently use libmaxminddb as a backend.
void mk_mmdbx_settings_set_asn_db_path(mk_mmdbx_settings_t *p,
                                       const char *v);

// mk_mmdbx_settings_set_ca_path sets path to CA bundle path.
void mk_mmdbx_settings_set_ca_path(mk_mmdbx_settings_t *p, const char *ca);

// mk_mmdbx_settings_delete deletes a settings instance.
void mk_mmdbx_settings_delete(mk_mmdbx_settings_t *p);

// mk_mmdbx_results_t contains the results.
typedef struct mk_mmdbx_results mk_mmdbx_results_t;

// mk_mmdbx_results_get_error returs the error that occurred.
mk_mmdbx_error_t mk_mmdbx_results_get_error(mk_mmdbx_results_t *p);

// mk_mmdbx_results_get_probe_ip returns the probe IP.
const char *mk_mmdbx_results_get_probe_ip(mk_mmdbx_results_t *p);

// mk_mmdbx_results_get_probe_asn returns the probe ASN.
const char *mk_mmdbx_results_get_probe_asn(mk_mmdbx_results_t *p);

// mk_mmdbx_results_get_probe_cc returns the probe CC.
const char *mk_mmdbx_results_get_probe_cc(mk_mmdbx_results_t *p);

// mk_mmdbx_results_get_probe_network_name returns the probe network name.
const char *mk_mmdbx_results_get_probe_network_name(mk_mmdbx_results_t *p);

// mk_mmdbx_results_get_bytes_sent get the bytes sent during the mmdbx lookup.
double mk_mmdbx_results_get_bytes_sent(mk_mmdbx_results_t *p);

// mk_mmdbx_results_get_bytes_sent get the bytes recv during the mmdbx lookup.
double mk_mmdbx_results_get_bytes_recv(mk_mmdbx_results_t *p);

// mk_mmdbx_results_get_bytes_sent get the mmdbx lookup logs.
const char *mk_mmdbx_results_get_logs(mk_mmdbx_results_t *p);

// mk_mmdbx_results_delete deletes a results instance.
void mk_mmdbx_results_delete(mk_mmdbx_results_t *p);

// mk_mmdbx_lookup resolves the probe IP, the probe ASN, the probe CC,
// and the probe network name using the specified |p| settings. The return
// value MAY be NULL under severe internal error conditions as well as when
// the provided |p| parameter is NULL. Note that you own the pointer that
// is returned by this function and must delete it when done.
mk_mmdbx_results_t *mk_mmdbx_lookup(const mk_mmdbx_settings_t *p);

#ifdef __cplusplus
}  // extern "C"

// mk_mmdbx_settings_deleter is a custom deleter for mk_mmdbx_settings_t.
struct mk_mmdbx_settings_deleter {
  void operator()(mk_mmdbx_settings_t *p) { mk_mmdbx_settings_delete(p); }
};

// mk_mmdbx_settings_uptr is syntactic sugar for using a settings object with
// RAII semantic when using this code from C++.
using mk_mmdbx_settings_uptr = std::unique_ptr<mk_mmdbx_settings_t,
                                               mk_mmdbx_settings_deleter>;

// mk_mmdbx_results_deleter is a custom deleter for mk_mmdbx_results_t.
struct mk_mmdbx_results_deleter {
  void operator()(mk_mmdbx_results_t *p) { mk_mmdbx_results_delete(p); }
};

// mk_mmdbx_results_uptr is syntactic sugar for using a results object with
// RAII semantic when using this code from C++.
using mk_mmdbx_results_uptr = std::unique_ptr<mk_mmdbx_results_t>;

// By default the implementation is not included. You can force it being
// included by providing the following definition to the compiler.
//
// If you're just into understanding the API, you can stop reading here.
#ifdef MK_MMDBX_INLINE_IMPL

#include <ctype.h>

#include <functional>
#include <string>

#include <maxminddb.h>

#include "libcurlx.h"

struct mk_mmdbx_settings {
  unsigned timeout = 7;
  std::string country_db_path;
  std::string asn_db_path;
  std::string ca_path;
};

mk_mmdbx_settings_t *mk_mmdbx_settings_new() {
  return new mk_mmdbx_settings;
}

void mk_mmdbx_settings_set_timeout(mk_mmdbx_settings_t *p, unsigned v) {
  if (p != nullptr) p->timeout = v;
}

void mk_mmdbx_settings_set_country_db_path(mk_mmdbx_settings_t *p,
                                           const char *v) {
  if (p != nullptr && v != nullptr) p->country_db_path = v;
}

void mk_mmdbx_settings_set_asn_db_path(mk_mmdbx_settings_t *p,
                                            const char *v) {
  if (p != nullptr && v != nullptr) p->asn_db_path = v;
}

void mk_mmdbx_settings_set_ca_path(mk_mmdbx_settings_t *p,
                                          const char *v) {
  if (p != nullptr && v != nullptr) p->ca_path = v;
}

void mk_mmdbx_settings_delete(mk_mmdbx_settings_t *p) {
  delete p;
}

struct mk_mmdbx_results {
  mk_mmdbx_error_t error = MK_MMDBX_ENONE;
  std::string probe_ip;
  std::string probe_asn;
  std::string probe_cc;
  std::string probe_network_name;
  std::string logs;
  double bytes_recv = 0.0;
  double bytes_sent = 0.0;
};

mk_mmdbx_error_t mk_mmdbx_results_get_error(mk_mmdbx_results_t *p) {
  return (p != nullptr) ? p->error : MK_MMDBX_EFAULT;
}

const char *mk_mmdbx_results_get_probe_ip(mk_mmdbx_results_t *p) {
  return (p != nullptr) ? p->probe_ip.c_str() : "";
}

const char *mk_mmdbx_results_get_probe_asn(mk_mmdbx_results_t *p) {
  return (p != nullptr) ? p->probe_asn.c_str() : "";
}

const char *mk_mmdbx_results_get_probe_cc(mk_mmdbx_results_t *p) {
  return (p != nullptr) ? p->probe_cc.c_str() : "";
}

const char *mk_mmdbx_results_get_probe_network_name(mk_mmdbx_results_t *p) {
  return (p != nullptr) ? p->probe_network_name.c_str() : "";
}

const char *mk_mmdbx_results_get_logs(mk_mmdbx_results_t *p) {
  return (p != nullptr) ? p->logs.c_str() : "";
}

double mk_mmdbx_results_get_bytes_recv(mk_mmdbx_results_t *p) {
  return (p != nullptr) ? p->bytes_recv : 0.0;
}

double mk_mmdbx_results_get_bytes_sent(mk_mmdbx_results_t *p) {
  return (p != nullptr) ? p->bytes_sent : 0.0;
}

void mk_mmdbx_results_delete(mk_mmdbx_results_t *p) {
  delete p;
}

static bool lookup_ip(const mk_mmdbx_settings_t *, mk_mmdbx_results_uptr &);
static bool lookup_cc(const mk_mmdbx_settings_t *, mk_mmdbx_results_uptr &);
static bool lookup_asn(const mk_mmdbx_settings_t *, mk_mmdbx_results_uptr &);

mk_mmdbx_results_t *mk_mmdbx_lookup(const mk_mmdbx_settings_t *p) {
  if (p == nullptr) return nullptr;
  mk_mmdbx_results_uptr r{new mk_mmdbx_results_t};
  // TODO(bassosimone): see if we really need a failure logic
  (void)lookup_ip(p, r);
  (void)lookup_cc(p, r);
  (void)lookup_asn(p, r);
  return r.release();
}

static bool parse_ip(const std::string &s, mk_mmdbx_results_uptr &r);

static bool lookup_ip(const mk_mmdbx_settings_t *p, mk_mmdbx_results_uptr &r) {
  mk_curlx_request_uptr req{mk_curlx_request_new()};
  if (!req) {
    r->error = MK_MMDBX_ENOMEM;
    return false;
  }
  mk_curlx_request_enable_http2(req.get());
  mk_curlx_request_set_url(req.get(), "https://geoip.ubuntu.com/lookup");
  mk_curlx_request_set_timeout(req.get(), p->timeout);
  if (!p->ca_path.empty()) {
    mk_curlx_request_set_ca_path(req.get(), p->ca_path.c_str());
  }
  mk_curlx_response_uptr res{mk_curlx_perform(req.get())};
  if (!res) {
    r->error = MK_MMDBX_ENOMEM;
    return false;
  }
  r->bytes_sent += mk_curlx_response_get_bytes_sent(res.get());
  r->bytes_recv += mk_curlx_response_get_bytes_recv(res.get());
  r->logs += mk_curlx_response_get_logs(res.get());
  if (mk_curlx_response_get_error(res.get()) != 0) {
    r->error = MK_MMDBX_ECURL;
    return false;
  }
  if (mk_curlx_response_get_status_code(res.get()) != 200) {
    r->error = MK_MMDBX_EHTTP;
    return false;
  }
  std::string body = mk_curlx_response_get_body(res.get());
  if (!parse_ip(body, r)) {
    r->logs += "Failed to parse IP\n";
    return false;
  }
  r->logs += "Successfully parsed IP: ";
  r->logs += r->probe_ip;
  r->logs += "\n";
  return true;
}

static bool parse_ip(const std::string &s, mk_mmdbx_results_uptr &r) {
  static const std::string open_tag = "<Ip>";
  static const std::string close_tag = "</Ip>";
  std::string input = s;  // Making a copy
  auto pos = input.find(open_tag);
  if (pos == std::string::npos) return false;
  input = input.substr(pos + open_tag.size());
  pos = input.find(close_tag);
  if (pos == std::string::npos) return false;
  input = input.substr(0, pos);
  for (auto ch : input) {
    if (isspace(ch)) continue;
    ch = (char)tolower(ch);
    auto ok = isdigit(ch) || (ch >= 'a' && ch <= 'f') || ch == '.' || ch == ':';
    if (!ok) return false;
    r->probe_ip += ch;
  }
  return true;
}

static bool lookup_mmdb_using_probe_ip(
    const std::string &path, mk_mmdbx_results_uptr &r,
    std::function<bool(MMDB_entry_s *)> fun);

#ifndef MK_MMDBX_MMDB_GET_VALUE
#define MK_MMDBX_MMDB_GET_VALUE MMDB_get_value
#endif

#ifndef MK_MMDBX_MMDB_OPEN
#define MK_MMDBX_MMDB_OPEN MMDB_open
#endif

#ifndef MK_MMDBX_MMDB_LOOKUP_STRING
#define MK_MMDBX_MMDB_LOOKUP_STRING MMDB_lookup_string
#endif

static bool lookup_cc(const mk_mmdbx_settings_t *p, mk_mmdbx_results_uptr &r) {
  return lookup_mmdb_using_probe_ip(
      p->country_db_path, r, [&](MMDB_entry_s *entry) {
        {
          MMDB_entry_data_s data{};
          auto mmdb_error = MK_MMDBX_MMDB_GET_VALUE(
              entry, &data, "registered_country", "iso_code", nullptr);
          if (mmdb_error != 0) {
            r->error = MK_MMDBX_EMMDBGETVALUE;
            r->logs += "MMDB_get_value() failed: ";
            r->logs += MMDB_strerror(mmdb_error);
            r->logs += "\n";
            return false;
          }
          if (!data.has_data || data.type != MMDB_DATA_TYPE_UTF8_STRING) {
            r->error = MK_MMDBX_ENODATAFORTYPE;
            r->logs += "MMDB_get_value() failed: no data for expected type";
            r->logs += "\n";
            return false;
          }
          r->probe_cc = std::string{data.utf8_string, data.data_size};
          r->logs += "Probe CC: ";
          r->logs += r->probe_cc;
          r->logs += "\n";
        }
        return true;
      });
}

static bool lookup_asn(const mk_mmdbx_settings_t *p, mk_mmdbx_results_uptr &r) {
  return lookup_mmdb_using_probe_ip(
      p->asn_db_path, r, [&](MMDB_entry_s *entry) {
        {
          MMDB_entry_data_s data{};
          auto mmdb_error = MK_MMDBX_MMDB_GET_VALUE(
              entry, &data, "autonomous_system_number", nullptr);
          if (mmdb_error != 0) {
            r->error = MK_MMDBX_EMMDBGETVALUE;
            r->logs += "MMDB_get_value() failed: ";
            r->logs += MMDB_strerror(mmdb_error);
            r->logs += "\n";
            return false;
          }
          if (!data.has_data || data.type != MMDB_DATA_TYPE_UINT32) {
            r->error = MK_MMDBX_ENODATAFORTYPE;
            r->logs += "MMDB_get_value() failed: no data for expected type";
            r->logs += "\n";
            return false;
          }
          r->probe_asn = std::string{"AS"} + std::to_string(data.uint32);
          r->logs += "Probe ASN: ";
          r->logs += r->probe_asn;
          r->logs += "\n";
        }
        {
          MMDB_entry_data_s data{};
          auto mmdb_error = MK_MMDBX_MMDB_GET_VALUE(
              entry, &data, "autonomous_system_organization", nullptr);
          if (mmdb_error != 0) {
            r->error = MK_MMDBX_EMMDBGETVALUE;
            r->logs += "MMDB_get_value() failed: ";
            r->logs += MMDB_strerror(mmdb_error);
            r->logs += "\n";
            return false;
          }
          if (!data.has_data || data.type != MMDB_DATA_TYPE_UTF8_STRING) {
            r->error = MK_MMDBX_ENODATAFORTYPE;
            r->logs += "MMDB_get_value() failed: no data for expected type";
            r->logs += "\n";
            return false;
          }
          r->probe_network_name = std::string{
              data.utf8_string, data.data_size};
          r->logs += "Probe Network Name: ";
          r->logs += r->probe_network_name;
          r->logs += "\n";
        }
        return true;
      });
}

static bool lookup_mmdb_using_probe_ip(
    const std::string &path, mk_mmdbx_results_uptr &r,
    std::function<bool(MMDB_entry_s *)> fun) {
  MMDB_s mmdb{};
  auto mmdb_error = MK_MMDBX_MMDB_OPEN(path.c_str(), MMDB_MODE_MMAP, &mmdb);
  if (mmdb_error != 0) {
    r->error = MK_MMDBX_EMMDBOPEN;
    r->logs += "MMDB_open() failed: ";
    r->logs += MMDB_strerror(mmdb_error);
    r->logs += "\n";
    return false;
  }
  auto rv = false;
  do {
    auto gai_error = 0;
    mmdb_error = 0;
    auto record = MK_MMDBX_MMDB_LOOKUP_STRING(&mmdb, r->probe_ip.c_str(),
                                              &gai_error, &mmdb_error);
    if (gai_error) {
      r->error = MK_MMDBX_EGETADDRINFO;
      r->logs += "MMDB_lookup_string() failed: ";
      r->logs += gai_strerror(gai_error);
      r->logs += "\n";
      break;
    }
    if (mmdb_error) {
      r->error = MK_MMDBX_EMMDBLOOKUPSTRING;
      r->logs += "MMDB_lookup_string() failed: ";
      r->logs += MMDB_strerror(gai_error);
      r->logs += "\n";
      break;
    }
    if (!record.found_entry) {
      r->error = MK_MMDBX_ENOTFOUND;
      r->logs += "MMDB_lookup_string() failed: no entry for probe_ip";
      r->logs += "\n";
      break;
    }
    rv = fun(&record.entry);
  } while (0);
  MMDB_close(&mmdb);
  return rv;
}

#endif  // MK_MMDBX_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_LIBMMDBX_LIBMMDBX_H
