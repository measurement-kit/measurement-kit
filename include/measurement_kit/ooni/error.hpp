// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_ERROR_HPP
#define MEASUREMENT_KIT_OONI_ERROR_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace ooni {

MK_DEFINE_ERR(MK_ERR_OONI(0), MissingRequiredInputFileError, "")
MK_DEFINE_ERR(MK_ERR_OONI(1), MissingCollectorBaseUrlError, "")
MK_DEFINE_ERR(MK_ERR_OONI(2), CannotOpenReportError, "")
MK_DEFINE_ERR(MK_ERR_OONI(3), MissingMandatoryKeyError, "")
MK_DEFINE_ERR(MK_ERR_OONI(4), InvalidMandatoryValueError, "")
MK_DEFINE_ERR(MK_ERR_OONI(5), MissingRequiredHostError, "")
MK_DEFINE_ERR(MK_ERR_OONI(6), MissingRequiredUrlError, "")
MK_DEFINE_ERR(MK_ERR_OONI(7), GeoipDatabaseOpenError, "")
MK_DEFINE_ERR(MK_ERR_OONI(8), GeoipCountryCodeLookupError, "")
MK_DEFINE_ERR(MK_ERR_OONI(9), GeoipCountryNameLookupError, "")
MK_DEFINE_ERR(MK_ERR_OONI(10), GeoipCityLookupError, "")
MK_DEFINE_ERR(MK_ERR_OONI(11), GeoipAsnLookupError, "")
MK_DEFINE_ERR(MK_ERR_OONI(12), CannotGetResourcesVersionError, "")
MK_DEFINE_ERR(MK_ERR_OONI(13), CannotGetResourcesManifestError, "")
MK_DEFINE_ERR(MK_ERR_OONI(14), CannotGetResourceError, "")
MK_DEFINE_ERR(MK_ERR_OONI(15), ResourceIntegrityError, "")

} // namespace mk
} // namespace ooni
#endif
