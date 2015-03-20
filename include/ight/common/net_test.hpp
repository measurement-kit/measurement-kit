/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_COMMON_NET_TEST_HPP
# define IGHT_COMMON_NET_TEST_HPP

///
/// \file ight/common/net_test.hpp
/// \brief Net class of all network tests.
///

#include <ight/common/constraints.hpp>
#include <ight/common/settings.hpp>

#include <functional>
#include <string>

namespace ight {
namespace common {
namespace net_test {

using namespace ight::common::constraints;
using namespace ight::common;

struct NetTest : public NonCopyable, public NonMovable {

    /*!
     * \brief Start iterating over the input.
     * \param func Callback called when we are done.
     */
    virtual void begin(std::function<void()> func) = 0;

    /*!
     * \brief Make sure that the report is written.
     * \param func Callback called when the report is written.
     */
    virtual void end(std::function<void()> func) = 0;

    /*!
     * \brief Return the unique identifier of this test.
     * \return Unique identifier of this test.
     */
    virtual unsigned long long identifier() {
        return (unsigned long long) this;
    }

    /// Default destructor
    virtual ~NetTest() {}
};

}}}
#endif
