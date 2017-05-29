// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_UTILS_HPP
#define MEASUREMENT_KIT_COMMON_UTILS_HPP

#include <measurement_kit/common/aaa_base.hpp>
#include <measurement_kit/common/error_or.hpp>

#include <list>
#include <regex>

namespace mk {

void timeval_now(timeval *);
double time_now();
void utc_time_now(tm *);
ErrorOr<std::string> timestamp(const tm *);
timeval *timeval_init(timeval *, double);
Error parse_iso8601_utc(std::string ts, std::tm *tmb);

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

std::string sha256_of(std::string input);

ErrorOr<std::vector<char>> slurpv(std::string path);
ErrorOr<std::string> slurp(std::string path);

bool startswith(std::string s, std::string p);
bool endswith(std::string s, std::string p);

} // namespace mk
#endif
