/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_NET_TRANSPORT_HPP
# define LIBIGHT_NET_TRANSPORT_HPP

//
// Generic transport (stream socket, SOCKSv5, etc.)
//

#include <functional>
#include <string>

#include "common/constraints.hpp"
#include "common/error.h"
#include "common/pointer.hpp"
#include "common/settings.hpp"

#include "net/buffer.hpp"

namespace ight {
namespace net {
namespace transport {

using namespace ight::common::constraints;
using namespace ight::common::pointer;
using namespace ight::common;

struct Transport : public NonMovable, public NonCopyable {

    Transport() {}

    virtual void on_connect(std::function<void()>) = 0;

    virtual void on_ssl(std::function<void()>) = 0;

    virtual void on_data(std::function<void(SharedPointer<IghtBuffer>)>) = 0;

    virtual void on_flush(std::function<void()>) = 0;

    virtual void on_error(std::function<void(IghtError)>) = 0;

    virtual void set_timeout(double) = 0;

    virtual void clear_timeout() = 0;

    virtual void send(const void*, size_t) = 0;

    virtual void send(std::string) = 0;

    virtual void send(SharedPointer<IghtBuffer>) = 0;

    virtual void send(IghtBuffer&) = 0;

    virtual void close() = 0;
};

SharedPointer<Transport> connect(Settings);

}}}  // namespace

#endif  // LIBIGHT_NET_TRANSPORT_HPP
