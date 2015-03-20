/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_COMMON_LOG_HPP
# define IGHT_COMMON_LOG_HPP

#include <functional>

void ight_warn(const char *, ...)
  __attribute__((format(printf, 1, 2)));

void ight_info(const char *, ...)
  __attribute__((format(printf, 1, 2)));

void ight_debug(const char *, ...)
  __attribute__((format(printf, 1, 2)));

void ight_set_verbose(int);

void ight_set_logger(std::function<void(const char *)>);

#endif
