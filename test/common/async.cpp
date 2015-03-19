/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/async.cpp's
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/common/async.hpp>
#include <ight/common/log.hpp>
#include <ight/common/pointer.hpp>
#include <ight/common/settings.hpp>

#include <ight/ooni/http_invalid_request_line.hpp>

#include <event2/thread.h>  // XXX
#include <unistd.h>  // XXX

using namespace ight::common::async;
using namespace ight::common::pointer;
using namespace ight::common;

using namespace ight::ooni::http_invalid_request_line;

TEST_CASE("The async engine works as expected") {
    evthread_use_pthreads();
    ight_set_verbose(1);
    Async async;
    async.run_test(SharedPointer<HTTPInvalidRequestLine>(
        new HTTPInvalidRequestLine(Settings{
            {"backend", "http://nexa.polito.it/"},
        })
    ));
    async.run_test(SharedPointer<HTTPInvalidRequestLine>(
        new HTTPInvalidRequestLine(Settings{
            {"backend", "http://www.google.com/"},
        })
    ));

    for (;;) {
        sleep(1);
    }
}
