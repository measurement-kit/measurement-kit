/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_COMMON_CONSTRAINTS_HPP
# define LIBIGHT_COMMON_CONSTRAINTS_HPP

namespace ight {
namespace common {
namespace constraints {

struct NonMovable {
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
    NonMovable() {}
};

struct NonCopyable {
    NonCopyable(NonCopyable&) = delete;
    NonCopyable& operator=(NonCopyable&) = delete;
    NonCopyable() {}
};

}}}  // namespaces
#endif  // LIBIGHT_COMMON_CONSTRAINTS_HPP
