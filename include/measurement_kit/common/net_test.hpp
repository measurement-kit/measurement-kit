// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_NET_TEST_HPP
#define MEASUREMENT_KIT_COMMON_NET_TEST_HPP

///
/// \brief Base class of all network tests.
///

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/settings.hpp>

#include <functional>
#include <string>

namespace measurement_kit {
namespace common {

/// \brief The generic network test
struct NetTest : public NonCopyable, public NonMovable {

  protected:
    Logger logger;

  public:
    /// \brief Set log function used by this test.
    virtual void set_log_function(std::function<void(const char *)> func) {
        logger.on_log(func);
    }

    /// \brief Make this test log verbose.
    virtual void set_log_verbose(int verbose) {
        logger.set_verbose(verbose);
    }

    /// \brief Start iterating over the input.
    /// \param func Callback called when we are done.
    virtual void begin(std::function<void()> func) = 0;

    /// \brief Make sure that report is written.
    /// \param func Callback invoked when report is written.
    virtual void end(std::function<void()> func) = 0;

    /// \brief Return the unique identifier of this test.
    virtual unsigned long long identifier() {
        return (unsigned long long) this;
    }

    /// \brief Default destructor.
    virtual ~NetTest() {}
};

}}
#endif
