// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/nettests/utils_impl.hpp"

namespace mk {
namespace nettests {

Error process_input_filepaths(std::deque<std::string> &input,
    const bool &needs_input, const std::list<std::string> &input_filepaths,
    const std::string &probe_cc, const Settings &options, Var<Logger> logger,
    std::function<void(const std::string &)> on_open_error,
    std::function<void(const std::string &)> on_io_error) {
    return process_input_filepaths_impl(input, needs_input, input_filepaths,
                                        probe_cc, options, logger,
                                        on_open_error, on_io_error);
}
} // namespace nettests
} // namespace mk
