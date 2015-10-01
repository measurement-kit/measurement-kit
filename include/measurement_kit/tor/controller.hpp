// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_TOR_CONTROLLER_HPP
#define MEASUREMENT_KIT_TOR_CONTROLLER_HPP

#include <functional>
#include <iosfwd>
#include <string>

#include <measurement_kit/common/error.hpp>

namespace measurement_kit {
namespace tor {

/// Tor controller class
class Controller {
  public:
    /// Constructor with custom address and port
    Controller(std::string socks_host, int socks_port,
               std::function<void()> start_tor)
        : host_(socks_host), port_(socks_port), start_tor_(start_tor) {}

    /// Default constructor
    Controller(std::function<void()> start_tor)
        : Controller("127.0.0.1", 9050, start_tor) {}

    /// Set callback called on completion
    void on_complete(std::function<void(common::Error)> func) {
        complete_fn_ = func;
    }

    /// Set callback called on progress
    void on_progress(std::function<void(int, std::string)> func) {
        progress_fn_ = func;
    }

    /// Start tor
    void start_tor() { start_tor_(); }

  private:
    std::function<void(common::Error)> complete_fn_;
    std::function<void(int, std::string)> progress_fn_;
    std::string host_;
    std::function<void()> start_tor_;
    int port_ = 0;
};

} // namespace tor
} // namespace measurement_kit
#endif
