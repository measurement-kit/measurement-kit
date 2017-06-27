// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef PRIVATE_KIT_COMMON_UTILS_HPP
#define PRIVATE_KIT_COMMON_UTILS_HPP

#include <measurement_kit/common.hpp>

struct timeval;

namespace mk {

std::string random_within_charset(std::string charset, size_t length);
std::string random_printable(size_t length);
std::string random_str(size_t length);
std::string random_str_uppercase(size_t length);
std::string random_choice(std::vector<std::string> inputs);
std::string randomly_capitalize(std::string input);

void dump_settings(Settings &s, std::string prefix, Var<Logger> logger);

double percentile(std::vector<double> v, double percent);

inline double median(std::vector<double> v) {
    return percentile(v, 0.5);
}

} // namespace mk
#endif
