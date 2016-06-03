// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_REPORT_ERROR_HPP
#define MEASUREMENT_KIT_REPORT_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace report {

MK_DEFINE_ERR(MK_ERR_REPORT(0), ReportAlreadyOpen, "")
MK_DEFINE_ERR(MK_ERR_REPORT(1), ReportNotOpen, "")
MK_DEFINE_ERR(MK_ERR_REPORT(2), ReportAlreadyClosed, "")
MK_DEFINE_ERR(MK_ERR_REPORT(3), ReportEofError, "")
MK_DEFINE_ERR(MK_ERR_REPORT(4), ReportIoError, "")
MK_DEFINE_ERR(MK_ERR_REPORT(5), ReportLogicalError, "")

} // namespace report
} // namespace mk
#endif
