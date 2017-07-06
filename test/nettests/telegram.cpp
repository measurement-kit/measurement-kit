// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;

TEST_CASE("Synchronous telegram test") {
    test::nettests::make_test<TelegramTest>()
        .run();
}

TEST_CASE("Asynchronous meek-fronted-requests test") {
    auto t = test::nettests::make_test<TelegramTest>();
    test::nettests::run_async(t);
}

#else
int main() {}
#endif
