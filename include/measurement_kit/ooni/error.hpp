// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_ERROR_HPP
#define MEASUREMENT_KIT_OONI_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace ooni {

MK_DEFINE_ERR(MK_ERR_OONI(0), CannotReadAnyInputFileError,
              "cannot_read_any_input_file")
MK_DEFINE_ERR(MK_ERR_OONI(1), MissingRequiredInputFileError,
              "missing_required_input_file")
MK_DEFINE_ERR(MK_ERR_OONI(2), MissingCollectorBaseUrlError,
              "missing_collector_base_url")
MK_DEFINE_ERR(MK_ERR_OONI(3), CannotOpenReportError, "cannot_open_report")
MK_DEFINE_ERR(MK_ERR_OONI(4), MissingMandatoryKeyError, "missing_mandatory_key")
MK_DEFINE_ERR(MK_ERR_OONI(5), InvalidMandatoryValueError,
              "invalid_mandatory_value")
MK_DEFINE_ERR(MK_ERR_OONI(6), MissingRequiredHostError, "missing_required_host")
MK_DEFINE_ERR(MK_ERR_OONI(7), MissingRequiredUrlError, "missing_required_url")
MK_DEFINE_ERR(MK_ERR_OONI(8), GeoipDatabaseOpenError,
              "cannot_open_geoip_database")
MK_DEFINE_ERR(MK_ERR_OONI(9), GeoipCountryCodeLookupError,
              "cannot_find_country_code")
MK_DEFINE_ERR(MK_ERR_OONI(10), GeoipCountryNameLookupError,
              "cannot_find_country_name")
MK_DEFINE_ERR(MK_ERR_OONI(11), GeoipCityLookupError, "cannot_find_city")
MK_DEFINE_ERR(MK_ERR_OONI(12), GeoipAsnLookupError, "cannot_find_asn")
MK_DEFINE_ERR(MK_ERR_OONI(13), CannotGetResourcesVersionError,
              "cannot_get_resources_version")
MK_DEFINE_ERR(MK_ERR_OONI(14), CannotGetResourcesManifestError,
              "cannot_get_resources_manifest")
MK_DEFINE_ERR(MK_ERR_OONI(15), CannotGetResourceError, "cannot_get_resource")
MK_DEFINE_ERR(MK_ERR_OONI(16), ResourceIntegrityError,
              "resource_checksum_failed")
MK_DEFINE_ERR(MK_ERR_OONI(17), BouncerCollectorNotFoundError,
              "bouncer_missing_collector")
MK_DEFINE_ERR(MK_ERR_OONI(18), BouncerTestHelperNotFoundError,
              "bouncer_missing_test_helper")
MK_DEFINE_ERR(MK_ERR_OONI(19), BouncerInvalidRequestError,
              "bouncer_invalid_request")
MK_DEFINE_ERR(MK_ERR_OONI(20), BouncerGenericError, "bouncer_generic_error")
MK_DEFINE_ERR(MK_ERR_OONI(21), BouncerValueNotFoundError,
              "bouncer_value_not_found")
MK_DEFINE_ERR(MK_ERR_OONI(22), HttpRequestError, "http_request_error")
MK_DEFINE_ERR(MK_ERR_OONI(23), RegexSearchError, "regex_search_error")
MK_DEFINE_ERR(MK_ERR_OONI(24), RegistryWrongUsernamePasswordError,
              "registry_wrong_username_password")
MK_DEFINE_ERR(MK_ERR_OONI(25), RegistryMissingUsernamePasswordError,
              "registry_missing_username_password")
MK_DEFINE_ERR(MK_ERR_OONI(26), MissingAuthenticationTokenError,
              "missing_authentication_token")
MK_DEFINE_ERR(MK_ERR_OONI(27), MissingRequiredValueError,
              "missing_required_value")
MK_DEFINE_ERR(MK_ERR_OONI(28), RegistryInvalidRequestError,
              "registry_invalid_request")
MK_DEFINE_ERR(MK_ERR_OONI(29), RegistryEmptyClientIdError,
              "registry_empty_client_id")

} // namespace mk
} // namespace ooni
#endif
