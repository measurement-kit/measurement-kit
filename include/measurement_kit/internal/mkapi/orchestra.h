// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKAPI_ORCHESTRA_H
#define MEASUREMENT_KIT_MKAPI_ORCHESTRA_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/// mkapi_orchestra_client_t is a client for OONI orchestra.
typedef struct mkapi_orchestra_client mkapi_orchestra_client_t;

/// mkapi_orchestra_result_t is the result of calling a OONI orchestra API.
typedef struct mkapi_orchestra_result mkapi_orchestra_result_t;

/// mkapi_orchestra_client_new creates a new orchestra client. Always returns a
/// valid pointer of which you have ownership. Aborts if allocation fails.
mkapi_orchestra_client_t *mkapi_orchestra_client_new(void);

/// mkapi_orchestra_client_set_available_bandwidth sets the @p bandwidth
/// available to @p client. Aborts if passed null arguments.
void mkapi_orchestra_client_set_available_bandwidth(
    mkapi_orchestra_client_t *client, const char *available_bandwidth);

/// mkapi_orchestra_client_set_device_token sets the @p device_token
/// for @p client. Aborts if passed null arguments.
void mkapi_orchestra_client_set_device_token(
    mkapi_orchestra_client_t *client, const char *device_token);

/// mkapi_orchestra_client_set_ca_bundle_path sets the @p ca_bundle_path
/// to be used by @p client. Aborts if passed null arguments.
void mkapi_orchestra_client_set_ca_bundle_path(
    mkapi_orchestra_client_t *client, const char *ca_bundle_path);

/// mkapi_orchestra_client_set_geoip_country_path sets the @p geoip_country_path
/// for @p client. Note that we expect a GeoLite2 MaxMindDB (i.e. `.mmdb`)
/// file here, not a legacy `.dat` file. Aborts if passed null arguments.
void mkapi_orchestra_client_set_geoip_country_path(
    mkapi_orchestra_client_t *client, const char *geoip_country_path);

/// mkapi_orchestra_client_set_geoip_asn_path sets the @p geoip_asn_path
/// for @p client. Note that we expect a GeoLite2 MaxMindDB (i.e. `.mmdb`)
/// file here, not a legacy `.dat` file. Aborts if passed null arguments.
void mkapi_orchestra_client_set_geoip_asn_path(
    mkapi_orchestra_client_t *client, const char *geoip_asn_path);

/// mkapi_orchestra_client_set_language sets the @p language for @p client. It
/// aborts if passed null arguments.
void mkapi_orchestra_client_set_language(
    mkapi_orchestra_client_t *client, const char *language);

/// mkapi_orchestra_client_set_network_type sets the @p network_type for @p
/// client. It aborts if passed null arguments.
void mkapi_orchestra_client_set_network_type(
    mkapi_orchestra_client_t *client, const char *network_type);

/// mkapi_orchestra_client_set_platform sets the @p platform for @p
/// client. It aborts if passed null arguments.
void mkapi_orchestra_client_set_platform(
    mkapi_orchestra_client_t *client, const char *platform);

/// mkapi_orchestra_client_set_probe_asn sets the @p probe_asn for @p
/// client. It aborts if passed null arguments.
void mkapi_orchestra_client_set_probe_asn(
    mkapi_orchestra_client_t *client, const char *probe_asn);

/// mkapi_orchestra_client_set_probe_cc sets the @p probe_cc for @p
/// client. It aborts if passed null arguments.
void mkapi_orchestra_client_set_probe_cc(
    mkapi_orchestra_client_t *client, const char *probe_cc);

/// mkapi_orchestra_client_set_probe_family sets the @p probe_family for @p
/// client. It aborts if passed null arguments.
void mkapi_orchestra_client_set_probe_family(
    mkapi_orchestra_client_t *client, const char *probe_family);

/// mkapi_orchestra_client_set_probe_timezone sets the @p probe_timezone for @p
/// client. It aborts if passed null arguments.
void mkapi_orchestra_client_set_probe_timezone(
    mkapi_orchestra_client_t *client, const char *probe_timezone);

/// mkapi_orchestra_client_set_registry_url sets the @p registry_url for @p
/// client. It aborts if passed null arguments.
void mkapi_orchestra_client_set_registry_url(
    mkapi_orchestra_client_t *client, const char *registry_url);

/// mkapi_orchestra_client_set_secrets_file sets the @p secrets_file for @p
/// client. It aborts if passed null arguments.
void mkapi_orchestra_client_set_secrets_file(
    mkapi_orchestra_client_t *client, const char *secrets_file);

/// mkapi_orchestra_client_set_software_name sets the @p software_name for @p
/// client. It aborts if passed null arguments.
void mkapi_orchestra_client_set_software_name(
    mkapi_orchestra_client_t *client, const char *software_name);

/// mkapi_orchestra_client_set_software_version sets the @p software_version
/// for @p client. It aborts if passed null arguments.
void mkapi_orchestra_client_set_software_version(
    mkapi_orchestra_client_t *client, const char *software_version);

/// mkapi_orchestra_client_add_supported_test sets adds @p supported_test
/// to @p client. It aborts if passed null arguments.
void mkapi_orchestra_client_add_supported_test(
    mkapi_orchestra_client_t *client, const char *supported_test);

/// mkapi_orchestra_client_set_timeout sets the timeout (in seconds). It
/// calls abort if the @p client argument is a null pointer.
void mkapi_orchestra_client_set_timeout(
    mkapi_orchestra_client_t *client, int64_t timeout);

/// mkapi_orchestra_client_sync sends the metadata configured in the @p
/// client to the OONI backend, registering this client if required. Always
/// returns a valid pointer. Aborts in case a memory allocation fails and
/// when the @p client argument is a null pointer.
mkapi_orchestra_result_t *mkapi_orchestra_client_sync(
    const mkapi_orchestra_client_t *client);

/// mkapi_orchestra_client_delete destroys @p client, which MAY be null.
void mkapi_orchestra_client_delete(mkapi_orchestra_client_t *client);

/// mkapi_orchestra_result_good returns true if the API call succeeded
/// and false otherwise. Aborts if @p result is a null pointer.
int64_t mkapi_orchestra_result_good(const mkapi_orchestra_result_t *result);

/// mkapi_orchestra_result_get_binary_logs returns the logs stored inside @p
/// result into @p base and @p count. Aborts if passed null arguments.
void mkapi_orchestra_result_get_binary_logs(
    const mkapi_orchestra_result_t *result,
    const uint8_t **base, size_t *count);

/// mkapi_orchestra_result_delete destroys @p result, which MAY be null.
void mkapi_orchestra_result_delete(mkapi_orchestra_result_t *result);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#include <memory>
#include <string>

/// mkapi_orchestra_client_deleter is a deleter for mkapi_orchestra_client_t.
struct mkapi_orchestra_client_deleter {
  void operator()(mkapi_orchestra_client_t *p) {
    mkapi_orchestra_client_delete(p);
  }
};

/// mkapi_orchestra_client_uptr is a unique ptr to mkapi_orchestra_client_t.
using mkapi_orchestra_client_uptr = std::unique_ptr<
  mkapi_orchestra_client_t, mkapi_orchestra_client_deleter>;

/// mkapi_orchestra_result_deleter is a deleter for mkapi_orchestra_result_t.
struct mkapi_orchestra_result_deleter {
  void operator()(mkapi_orchestra_result_t *p) {
    mkapi_orchestra_result_delete(p);
  }
};

/// mkapi_orchestra_result_uptr is a unique ptr to mkapi_orchestra_result_t.
using mkapi_orchestra_result_uptr = std::unique_ptr<
  mkapi_orchestra_result_t, mkapi_orchestra_result_deleter>;

/// mkapi_orchestra_result_moveout_logs moves logs out of @p result.
std::string mkapi_orchestra_result_moveout_logs(
    mkapi_orchestra_result_uptr &result);

#endif  // MEASUREMENT_KIT_MK_ORCHESTRA_H
