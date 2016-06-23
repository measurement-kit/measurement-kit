// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_UTILS_HPP
#define MEASUREMENT_KIT_COMMON_UTILS_HPP

#include <sys/time.h>
#include <measurement_kit/common/error_or.hpp>
#include <list>
#include <string>
#include <regex>

namespace mk {

void timeval_now(timeval *);
double time_now();
void utc_time_now(struct tm *);
ErrorOr<std::string> timestamp(const struct tm *);
timeval *timeval_init(timeval *, double);

template <typename T=std::list<std::string>>
T split(std::string s, std::string pattern = "\\s+") {
    // See <http://stackoverflow.com/questions/9435385/>
    // passing -1 as the submatch index parameter performs splitting
    std::regex re{pattern};
    std::sregex_token_iterator
        first{s.begin(), s.end(), re, -1},
        last;
    return {first, last};
}


} // namespace mk
#endif
