// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_SSL_VERIFY_HPP
#define SRC_NET_SSL_VERIFY_HPP

#include <measurement_kit/common.hpp>
#include <openssl/x509v3.h>

namespace mk {
namespace net {

Error ssl_validate_hostname(std::string hostname, const X509 *server_cert);

}
}
#endif
