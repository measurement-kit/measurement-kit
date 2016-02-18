// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_TOR_HPP
#define SRC_TOR_HPP

#include <functional>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/net/transport.hpp>
#include <string>

namespace mk {

// Forward declaration
class Error;
class Poller;
namespace net {
class Buffer;
}

namespace tor {

class Control {
  public:
    Poller *poller;
    net::Transport transport;

    // Alternative to using Var<Buffer> here is to modify Buffer to make
    // most of its methods constant so we can use them in lambdas
    Var<net::Buffer> buffer;
};

/// Connect to the control port and authenticate.
/// \param callback Callback called on error or when connected.
/// \param address Optional address to connect to.
/// \param port Optional port to connect to.
/// \param poller Optional poller to use.
/// \param logger Optional logger to use.
void authenticate(std::function<void(Error, Control)> callback,
                  std::string address = "127.0.0.1", std::string port = "9051",
                  Poller *poller = Poller::global(),
                  Logger *logger = Logger::global());

/// Issues the 'SETCONF DisableNetwork=BOOL' command
/// \param ctrl The control connection
/// \param disable True to disable and false to enable
/// \param cb Callback called on error or success.
void setconf_disable_network(Control ctrl, bool disable,
                             std::function<void(Error)>);

/// Issues the 'GETINFO status/bootstrap-phase' command
/// \param control The control connection.
/// \param cb Callback called on error or success.
void getinfo_status_bootstrap_phase(Control, std::function<void(Error, int)>);

} // namespace tor
} // namespace mk
#endif
