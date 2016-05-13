// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_NET_TEST_HPP
#define MEASUREMENT_KIT_COMMON_NET_TEST_HPP

#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/common/settings.hpp>

#include <functional>

namespace mk {

class NetTest : public NonCopyable, public NonMovable {
  public:
    virtual void on_log(Delegate<void(uint32_t, const char *)> func) {
        logger->on_log(func);
    }
    virtual void set_verbosity(uint32_t level) { logger->set_verbosity(level); }

    virtual void begin(std::function<void()> func) = 0;
    virtual void end(std::function<void()> func) = 0;

    virtual unsigned long long identifier() { return (unsigned long long)this; }

    NetTest(Settings options) : options(options) {}
    virtual ~NetTest();

    Var<Logger> logger = Logger::make();
    Var<Reactor> reactor = Reactor::make();
    Settings options;
};

} // namespace mk
#endif
