// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_UTILS_HPP
#define MEASUREMENT_KIT_COMMON_UTILS_HPP

#include <sys/time.h>
#include <measurement_kit/common/error_or.hpp>
#include <string>

namespace mk {

void timeval_now(timeval *);
double time_now();
void utc_time_now(struct tm *);
ErrorOr<std::string> timestamp(const struct tm *);
timeval *timeval_init(timeval *, double);

} // namespace mk
#endif
