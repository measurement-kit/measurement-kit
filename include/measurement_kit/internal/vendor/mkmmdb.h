// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKMMDB_H
#define MEASUREMENT_KIT_MKMMDB_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/// mkmmdb_t is a MMDB database.
typedef struct mkmmdb mkmmdb_t;

/// mkmmdb_open_nonnull opens a database pointing to @p path. It always returns
/// a valid pointer. Call mkmmdb_good to understand whether the database was
/// successfully openned or not. This function will call abort when
/// the @p path argument is a null pointer.
mkmmdb_t *mkmmdb_open_nonnull(const char *path);

/// mkmmdb_good returns true if the database has been successfully open
/// and false otherwise. This function aborts if @p mmdb is null.
int64_t mkmmdb_good(mkmmdb_t *mmdb);

/// mkmmdb_lookup_cc returns the country code of @p ip using the
/// @p mmdb database, or the empty string on failure. The returned string
/// will be valid until @p mmdb is valid _and_ you don't call other
/// functions using the same @p mmdb instance. This function calls
/// abort if passed null parameters.
const char *mkmmdb_lookup_cc(mkmmdb_t *mmdb, const char *ip);

/// mkmmdb_lookup_asn is like mkmmdb_lookup_cc but returns
/// the ASN on success and zero on failure. Also this function
/// calls abort if passed null parameters.
int64_t mkmmdb_lookup_asn(mkmmdb_t *mmdb, const char *ip);

/// mkmmdb_lookup_org is like mkmmdb_lookup_cc but returns
/// the organization bound to @p ip on success, the empty string on
/// failure. The returned string will be valid until @p mmdb is
/// valid _and_ you don't call other lookup APIs using the
/// same @p mmdb instance. This function calls abort if
/// passed a null @p mmdb or a null @p ip.
const char *mkmmdb_lookup_org(mkmmdb_t *mmdb, const char *ip);

/// mkmmdb_get_last_lookup_logs returns the logs of the last lookup
/// or an empty string. Calls abort if @p mmdb is null.
const char *mkmmdb_get_last_lookup_logs(mkmmdb_t *mmdb);

/// mkmmdb_close closes @p mmdb. Note that @p mmdb MAY be null.
void mkmmdb_close(mkmmdb_t *mmdb);

#ifdef __cplusplus
}  // extern "C"

#include <memory>
#include <string>

/// mkmmdb_deleter is a deleter for mkmmdb_t.
struct mkmmdb_deleter {
  void operator()(mkmmdb_t *p) { mkmmdb_close(p); }
};

/// mkmmdb_uptr is a unique pointer to mkmmdb_t.
using mkmmdb_uptr = std::unique_ptr<mkmmdb_t, mkmmdb_deleter>;

// By default the implementation is not included. You can force it being
// included by providing the following definition to the compiler.
//
// If you're just into understanding the API, you can stop reading here.
#ifdef MKMMDB_INLINE_IMPL

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif

#include <functional>
#include <sstream>

#include <maxminddb.h>

// mkMMDB_s_deleter is a deleter for a MMDB_s pointer.
struct mkMMDB_s_deleter {
  void operator()(MMDB_s *p) {
    MMDB_close(p);
    delete p;
  }
};

// mkMMDB_s_uptr is a unique pointer to MMDB_s.
using mkMMDB_s_uptr = std::unique_ptr<MMDB_s, mkMMDB_s_deleter>;

// mkmmdb is a MMDB database.
struct mkmmdb {
  // mmdbs is a unique pointer to the real database instance.
  mkMMDB_s_uptr mmdbs;
  // last_lookup_result is the place where we save the latest lookup result.
  std::string last_lookup_result;
  // last_lookup_logs contains the logs of the last lookup.
  std::string last_lookup_logs;
};

#ifndef MKMMDB_MMDB_OPEN
// MKMMDB_MMDB_OPEN allows to mock MMDB_open
#define MKMMDB_MMDB_OPEN MMDB_open
#endif

#ifndef MKMMDB_ABORT
// MKMMDB_ABORT allows to mock abort
#define MKMMDB_ABORT abort
#endif

mkmmdb_t *mkmmdb_open_nonnull(const char *path) {
  if (path == nullptr) {
    MKMMDB_ABORT();
  }
  mkmmdb_uptr mmdb{new mkmmdb_t};
  mmdb->mmdbs.reset(new MMDB_s);
  if (MKMMDB_MMDB_OPEN(path, MMDB_MODE_MMAP, mmdb->mmdbs.get()) != 0) {
    mmdb->mmdbs = nullptr;
  }
  return mmdb.release();
}

int64_t mkmmdb_good(mkmmdb_t *mmdb) {
  if (mmdb == nullptr) {
    MKMMDB_ABORT();
  }
  return mmdb->mmdbs != nullptr;
}

#ifndef MKMMDB_MMDB_GET_VALUE
// MKMMDB_MMDB_GET_VALUE allows to mock MMDB_get_value
#define MKMMDB_MMDB_GET_VALUE MMDB_get_value
#endif

#ifndef MKMMDB_MMDB_LOOKUP_STRING
// MKMMDB_MMDB_LOOKUP_STRING allows to mock MMDB_lookup_string
#define MKMMDB_MMDB_LOOKUP_STRING MMDB_lookup_string
#endif

static void mkmmdb_lookup_mmdb(
    mkmmdb_t *mmdb, const std::string &ip,
    std::function<void(MMDB_entry_s *)> fun) {
  if (mmdb == nullptr || fun == nullptr) {
    MKMMDB_ABORT();
  }
  {
    mmdb->last_lookup_logs += "Start lookup for: ";
    mmdb->last_lookup_logs += ip;
    mmdb->last_lookup_logs += "\n";
  }
  if (mmdb->mmdbs == nullptr) {
    mmdb->last_lookup_logs += "The database is not open.\n";
    return;
  }
  auto gai_error = 0;
  auto mmdb_error = 0;
  auto record = MKMMDB_MMDB_LOOKUP_STRING(mmdb->mmdbs.get(), ip.c_str(),
                                          &gai_error, &mmdb_error);
  if (gai_error != 0) {
    mmdb->last_lookup_logs += "mmdb_lookup_string: ";
    mmdb->last_lookup_logs += gai_strerror(gai_error);
    mmdb->last_lookup_logs += "\n";
    return;
  }
  if (mmdb_error != 0) {
    mmdb->last_lookup_logs += "mmdb_lookup_string: ";
    mmdb->last_lookup_logs += MMDB_strerror(mmdb_error);
    mmdb->last_lookup_logs += "\n";
    return;
  }
  if (!record.found_entry) {
    mmdb->last_lookup_logs += "mmdb_lookup_string: entry not found.\n";
    return;
  }
  fun(&record.entry);
}

const char *mkmmdb_lookup_cc(mkmmdb_t *mmdb, const char *ip) {
  if (mmdb == nullptr || ip == nullptr) {
    MKMMDB_ABORT();
  }
  mmdb->last_lookup_result = "";
  mmdb->last_lookup_logs = "";
  mkmmdb_lookup_mmdb(
      mmdb, ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKMMDB_MMDB_GET_VALUE(
            entry, &data, "registered_country", "iso_code", nullptr);
        if (mmdb_error != 0) {
          mmdb->last_lookup_logs += "mmdb_get_value: ";
          mmdb->last_lookup_logs += MMDB_strerror(mmdb_error);
          mmdb->last_lookup_logs += "\n";
          return;
        }
        if (!data.has_data) {
          mmdb->last_lookup_logs += "mmdb_get_value: no data for entry.\n";
          return;
        }
        if (data.type != MMDB_DATA_TYPE_UTF8_STRING) {
          mmdb->last_lookup_logs += "mmdb_get_value: unexpected data type.\n";
          return;
        }
        mmdb->last_lookup_result = std::string{
          data.utf8_string, data.data_size};
      });
  return mmdb->last_lookup_result.c_str();
}

int64_t mkmmdb_lookup_asn(mkmmdb_t *mmdb, const char *ip) {
  if (mmdb == nullptr || ip == nullptr) {
    MKMMDB_ABORT();
  }
  mmdb->last_lookup_logs = "";
  int64_t rv = {};
  mkmmdb_lookup_mmdb(
      mmdb, ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKMMDB_MMDB_GET_VALUE(
            entry, &data, "autonomous_system_number", nullptr);
        if (mmdb_error != 0) {
          mmdb->last_lookup_logs += "mmdb_get_value: ";
          mmdb->last_lookup_logs += MMDB_strerror(mmdb_error);
          mmdb->last_lookup_logs += "\n";
          return;
        }
        if (!data.has_data) {
          mmdb->last_lookup_logs += "mmdb_get_value: no data for entry.\n";
          return;
        }
        if (data.type != MMDB_DATA_TYPE_UINT32) {
          mmdb->last_lookup_logs += "mmdb_get_value: unexpected data type.\n";
          return;
        }
        rv = data.uint32;
      });
  return rv;
}

const char *mkmmdb_lookup_org(mkmmdb_t *mmdb, const char *ip) {
  if (mmdb == nullptr || ip == nullptr) {
    MKMMDB_ABORT();
  }
  mmdb->last_lookup_logs = "";
  mmdb->last_lookup_result = "";
  mkmmdb_lookup_mmdb(
      mmdb, ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKMMDB_MMDB_GET_VALUE(
            entry, &data, "autonomous_system_organization", nullptr);
        if (mmdb_error != 0) {
          mmdb->last_lookup_logs += "mmdb_get_value: ";
          mmdb->last_lookup_logs += MMDB_strerror(mmdb_error);
          mmdb->last_lookup_logs += "\n";
          return;
        }
        if (!data.has_data) {
          mmdb->last_lookup_logs += "mmdb_get_value: no data for entry.\n";
          return;
        }
        if (data.type != MMDB_DATA_TYPE_UTF8_STRING) {
          mmdb->last_lookup_logs += "mmdb_get_value: unexpected data type.\n";
          return;
        }
        mmdb->last_lookup_result = std::string{
          data.utf8_string, data.data_size};
      });
  return mmdb->last_lookup_result.c_str();
}

const char *mkmmdb_get_last_lookup_logs(mkmmdb_t *mmdb) {
  if (mmdb == nullptr) {
    MKMMDB_ABORT();
  }
  return mmdb->last_lookup_logs.c_str();
}

void mkmmdb_close(mkmmdb_t *mmdb) { delete mmdb; }

#endif  // MKMMDB_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKMMDB_H
