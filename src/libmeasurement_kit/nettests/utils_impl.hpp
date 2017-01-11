// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NETTESTS_UTILS_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_NETTESTS_UTILS_IMPL_HPP

#include "../nettests/utils.hpp"

#include <measurement_kit/ooni.hpp>

#include <fstream>
#include <random>

namespace mk {
namespace nettests {

static std::ifstream open_file_(const std::string &path) {
    return std::ifstream{path};
}

static bool readline_(std::ifstream &input, std::string &line) {
    return static_cast<bool>(std::getline(input, line));
}

template <MK_MOCK(open_file_), MK_MOCK(readline_)>
ErrorOr<std::deque<std::string>>
process_input_filepaths_impl(const Settings &options, const bool &needs_input,
                             const std::list<std::string> &input_filepaths,
                             const std::string &probe_cc, Var<Logger> logger) {
    std::deque<std::string> inputs;
    if (needs_input) {
        if (input_filepaths.size() <= 0) {
            logger->warn("at least an input file is required");
            return ooni::MissingRequiredInputFileError();
        }
        std::string probe_cc_lowercase;
        for (auto &c : probe_cc) {
            /*
             * Note: in general the snippet below is not
             * so good because it does not work for UTF-8
             * and the like but here we are converting
             * country codes which are always ASCII.
             */
            probe_cc_lowercase += std::tolower(c);
        }
        for (auto input_filepath : input_filepaths) {
            input_filepath = std::regex_replace(input_filepath,
                                                std::regex{R"(\$\{probe_cc\})"},
                                                probe_cc_lowercase);
            std::ifstream input_generator = open_file_(input_filepath);
            if (!input_generator.good()) {
                logger->warn("cannot open input file");
                continue;
            }
            std::string next_input;
            while ((readline_(input_generator, next_input))) {
                inputs.push_back(next_input);
            }
            if (!input_generator.eof()) {
                logger->warn("I/O error reading input file");
                continue;
            }
        }
        if (inputs.size() <= 0) {
            logger->warn("no specified input file could be read");
            return ooni::CannotReadAnyInputFileError();
        }
        ErrorOr<bool> shuffle =
            options.get_noexcept<bool>("randomize_input", true);
        if (!shuffle) {
            logger->warn("invalid 'randomize_input' option");
            return shuffle.as_error();
        }
        if (*shuffle) {
            // http://en.cppreference.com/w/cpp/algorithm/shuffle:
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(inputs.begin(), inputs.end(), g);
        }
    } else {
        // Empty string to call main() just once
        inputs.push_back("");
    }
    return inputs;
}

} // namespace nettests
} // namespace mk
#endif
