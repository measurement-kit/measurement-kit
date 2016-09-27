// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_REPORT_ERROR_HPP
#define MEASUREMENT_KIT_REPORT_ERROR_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace report {

MK_DEFINE_ERR(MK_ERR_REPORT(0), ReportAlreadyOpenError, "")
MK_DEFINE_ERR(MK_ERR_REPORT(1), ReportNotOpenError, "")
MK_DEFINE_ERR(MK_ERR_REPORT(2), ReportAlreadyClosedError, "")
MK_DEFINE_ERR(MK_ERR_REPORT(3), ReportEofError, "")
MK_DEFINE_ERR(MK_ERR_REPORT(4), ReportIoError, "")
MK_DEFINE_ERR(MK_ERR_REPORT(5), ReportLogicalError, "")
MK_DEFINE_ERR(MK_ERR_REPORT(6), DuplicateEntrySubmitError, "")

} // namespace report
} // namespace mk
#endif
