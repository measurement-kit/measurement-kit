// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_REPORT_ERROR_HPP
#define SRC_LIBMEASUREMENT_KIT_REPORT_ERROR_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace report {

MK_DEFINE_ERR(MK_ERR_REPORT(0), ReportAlreadyOpenError, "report_already_open")
MK_DEFINE_ERR(MK_ERR_REPORT(1), ReportNotOpenError, "report_not_open")
MK_DEFINE_ERR(MK_ERR_REPORT(2), ReportAlreadyClosedError, "report_already_closed")
MK_DEFINE_ERR(MK_ERR_REPORT(3), ReportEofError, "report_eof_error")
MK_DEFINE_ERR(MK_ERR_REPORT(4), ReportIoError, "report_io_error")
MK_DEFINE_ERR(MK_ERR_REPORT(5), ReportLogicalError, "report_formatting_error")
MK_DEFINE_ERR(MK_ERR_REPORT(6), DuplicateEntrySubmitError, "duplicate_entry_submitted")
MK_DEFINE_ERR(MK_ERR_REPORT(7), MissingReportIdError, "missing_report_id")
MK_DEFINE_ERR(MK_ERR_REPORT(8), MultipleReportIdsError, "multiple_inconsistent_report_ids")

} // namespace report
} // namespace mk
#endif
