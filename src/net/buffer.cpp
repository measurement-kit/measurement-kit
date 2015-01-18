/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <arpa/inet.h>

#include "net/buffer.hpp"

void IghtBuffer::write_uint8(uint8_t num) {
    write(&num, sizeof (num));
}

void IghtBuffer::write_uint16(uint16_t num) {
    num = htons(num);
    write(&num, sizeof (num));
}

void IghtBuffer::write_uint32(uint32_t num) {
    num = htonl(num);
    write(&num, sizeof (num));
}
