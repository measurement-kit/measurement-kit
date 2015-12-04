// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_NET_TEST_HPP
#define MEASUREMENT_KIT_COMMON_NET_TEST_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/logger.hpp>

#include <functional>

namespace mk {

/// The generic network test
class NetTest : public NonCopyable, public NonMovable {
  public:
    /// Set log function used by this test.
    virtual void on_log(std::function<void(const char *)> func) {
        logger.on_log(func);
    }

    /// Make this test log verbose.
    virtual void set_verbose(int verbose) { logger.set_verbose(verbose); }

    /// Start iterating over the input.
    /// \param func Callback called when we are done.
    virtual void begin(std::function<void()> func) = 0;

    /// Make sure that report is written.
    /// \param func Callback invoked when report is written.
    virtual void end(std::function<void()> func) = 0;

    /// Return the unique identifier of this test.
    virtual unsigned long long identifier() { return (unsigned long long)this; }

    /// Default destructor.
    virtual ~NetTest();

  protected:
    Logger logger;
};

} // namespace mk
#endif
