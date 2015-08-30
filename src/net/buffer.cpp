// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/net/buffer.hpp>

#include <arpa/inet.h>

namespace measurement_kit {
namespace net {

void Buffer::write_uint8(uint8_t num) {
    write(&num, sizeof (num));
}

void Buffer::write_uint16(uint16_t num) {
    num = htons(num);
    write(&num, sizeof (num));
}

void Buffer::write_uint32(uint32_t num) {
    num = htonl(num);
    write(&num, sizeof (num));
}

} // namespace net
} // namespace measurement_kit
