// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NEUBOT_ERROR_HPP
#define MEASUREMENT_KIT_NEUBOT_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace neubot {

MK_DEFINE_ERR(MK_ERR_NEUBOT(0), TooManyNegotiationsError, "")
MK_DEFINE_ERR(MK_ERR_NEUBOT(1), NegativeTimeError, "")

} // namespace neubot
} // namespace mk

#endif
