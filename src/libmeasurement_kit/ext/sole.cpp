// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
// =============================================================
// Derivative work of r-lyeh/sole@c61c49f10d.
// See NOTICE for original license.

#include "src/libmeasurement_kit/ext/sole.hpp"

#include <iomanip>
#include <random>
#include <sstream>

namespace mk {
namespace sole {

std::string uuid::str() {
    std::stringstream ss;
    ss << std::hex << std::nouppercase << std::setfill('0');

    uint32_t a = (ab >> 32);
    uint32_t b = (ab & 0xFFFFFFFF);
    uint32_t c = (cd >> 32);
    uint32_t d = (cd & 0xFFFFFFFF);

    ss << std::setw(8) << (a) << '-';
    ss << std::setw(4) << (b >> 16) << '-';
    ss << std::setw(4) << (b & 0xFFFF) << '-';
    ss << std::setw(4) << (c >> 16) << '-';
    ss << std::setw(4) << (c & 0xFFFF);
    ss << std::setw(8) << d;

    return ss.str();
}

uuid uuid4() {
    std::random_device rd;
    std::uniform_int_distribution<uint64_t> dist(0, (uint64_t)(~0));
    uuid my;

    my.ab = dist(rd);
    my.cd = dist(rd);

    /* The version 4 UUID is meant for generating UUIDs from truly-random or
       pseudo-random numbers.

       The algorithm is as follows:

       o  Set the four most significant bits (bits 12 through 15) of the
          time_hi_and_version field to the 4-bit version number from
          Section 4.1.3.

       o  Set the two most significant bits (bits 6 and 7) of the
          clock_seq_hi_and_reserved to zero and one, respectively.

       o  Set all the other bits to randomly (or pseudo-randomly) chosen
          values.

       See <https://tools.ietf.org/html/rfc4122#section-4.4>. */
    my.ab = (my.ab & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    my.cd = (my.cd & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

    return my;
}

} // namespace sole
} // namespace mk
