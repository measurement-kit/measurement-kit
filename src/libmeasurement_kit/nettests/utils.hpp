// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NETTESTS_UTILS_HPP
#define SRC_LIBMEASUREMENT_KIT_NETTESTS_UTILS_HPP

#include <measurement_kit/common.hpp>

#include <deque>

namespace mk {
namespace nettests {

/**
 * @brief Generates the list of entries to be tested.
 *
 * @param needs_input If this value is false, the function returns just a
 * single, empty entry; otherwise, the function reads and returns as entries
 * all lines of the files provided as argument.
 *
 * @param input_filepaths When input is required, files to read; a missing
 * or non-readable file is not considered fatal and a warning will be
 * printed in such case, using the logger.
 *
 * @param probe_cc Country code of the probe: if a file path contains the
 * "${probe_cc}" string, this is expanded to the country code in lower case.
 *
 * @param options If the boolean "randomize_input" option is true, this
 * function randomizes the list of entries before returning it.
 *
 * @param logger Used to print log messages.
 *
 * @param on_open_error In case it is not possible to open a file, and
 * the function is not `nullptr`, the file name that failed is passed to
 * this function.
 *
 * @param on_io_error In case reading a file fails with I/O error, and
 * the function is not `nullptr`, the file name that failed is passed to
 * this function.
 *
 * @returns List of entries to test on sucess, error on failure. The
 * following errors will be returned:
 *
 * - MissingRequiredInputFileError if needs_input is true and no input
 *   file is passed through the input_filepaths argument
 *
 * - CannotReadAnyInputFileError if needs_input is true and the list of
 *   entries to be tested is empty after processing all input files
 *
 * - the error returned when trying to convert to boolean the value of
 *   "randomize_input" setting (e.g. ValueError)
 */
ErrorOr<std::deque<std::string>> process_input_filepaths(
        const bool &needs_input,
        const std::list<std::string> &input_filepaths,
        const std::string &probe_cc,
        const Settings &options,
        Var<Logger> logger,
        std::function<void(const std::string &)> on_open_error,
        std::function<void(const std::string &)> on_io_error
);

} // namespace nettests
} // namespace mk
#endif
