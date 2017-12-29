// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/utils_impl.hpp"

namespace mk {
namespace nettests {

SharedPtr<std::istream> open_file_(const std::string &path) {
    return SharedPtr<std::istream>{new std::ifstream{path}};
}

bool readline_(std::istream &input, std::string &line) {
    return static_cast<bool>(std::getline(input, line));
}

void randomize_input_(std::deque<std::string> &inputs) {
    // See http://en.cppreference.com/w/cpp/algorithm/shuffle
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(inputs.begin(), inputs.end(), g);
}

Error process_input_filepaths(std::deque<std::string> &input,
    const bool &needs_input, const std::list<std::string> &input_filepaths,
    const std::string &probe_cc, const Settings &options, SharedPtr<Logger> logger,
    std::function<void(const std::string &)> on_open_error,
    std::function<void(const std::string &)> on_io_error) {
    return process_input_filepaths_impl(input, needs_input, input_filepaths,
                                        probe_cc, options, logger,
                                        on_open_error, on_io_error);
}
} // namespace nettests
} // namespace mk
