// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/utils.hpp"
#include "private/common/worker.hpp"
#include "private/ooni/orchestrate_impl.hpp"

#include <future>

using namespace mk;
using namespace mk::ooni;
using namespace mk::ooni::orchestrate;

TEST_CASE("Auth::load() works correctly") {
    const std::string fname = "orchestrator_dummy.json";
    Auth auth;

    SECTION("with a nonexistent file") {
        REQUIRE(auth.load("/nonexistent") != NoError());
    }

    SECTION("with invalid JSON") {
        REQUIRE(overwrite_file(fname, "{") == NoError());
        REQUIRE(auth.load(fname) != NoError());
    }

    SECTION("with missing username field") {
        REQUIRE(overwrite_file(fname, "{}") == NoError());
        REQUIRE(auth.load(fname) != NoError());
    }

    SECTION("with missing password field") {
        REQUIRE(overwrite_file(fname, "{\"username\": \"xo\"}") == NoError());
        REQUIRE(auth.load(fname) != NoError());
    }

    SECTION("with good input") {
        [&]() {
            Auth auth;
            auth.auth_token = "{TOKEN}";
            auth.logged_in = true;
            auth.expiry_time = "fff";
            auth.username = "xo";
            auth.password = "xo";
            REQUIRE(overwrite_file(fname, auth.dumps()) == NoError());
        }();
        REQUIRE(auth.load(fname) == NoError());
        REQUIRE(auth.auth_token == "{TOKEN}");
        REQUIRE(auth.logged_in == true);
        REQUIRE(auth.expiry_time == "fff");
        REQUIRE(auth.username == "xo");
        REQUIRE(auth.password == "xo");
    }
}

TEST_CASE("Auth::dump() works correctly") {
    const std::string fname = "orchestrator_dummy.json";
    Auth auth;

    SECTION("with nonexistent file") {
        REQUIRE(auth.dump("/nonexistent/nonexistent") != NoError());
    }

    SECTION("with existent file") {
        auth.username = auth.password = "xo";
        REQUIRE(auth.dump(fname) == NoError());
        Json data = Json::parse(*slurp(fname));
        REQUIRE(data["username"] == "xo");
        REQUIRE(data["password"] == "xo");
    }
}

template <typename F> std::string make_time_(F &&f) {
    std::time_t t = f(std::time(nullptr));
    if (t == (std::time_t)-1) {
        throw std::runtime_error("std::time() failed");
    }
    std::tm ttm{};
    if (gmtime_r(&t, &ttm) == nullptr) {
        throw std::runtime_error("gmtime_r() failed");
    }
    std::stringstream ss;
    ss << std::put_time(&ttm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

TEST_CASE("Auth::is_valid() works correctly") {
    Auth auth;

    SECTION("When we are not logged in") {
        REQUIRE(auth.is_valid(Logger::global()) == false);
    }

    SECTION("When the token is empty") {
        auth.logged_in = true;
        REQUIRE(auth.is_valid(Logger::global()) == false);
    }

    SECTION("When we are logged in and time is expired") {
        auth.expiry_time = make_time_([](time_t t) { return t - 60; });
        auth.logged_in = true;
        auth.auth_token = "{TOKEN}";
        REQUIRE(auth.is_valid(Logger::global()) == false);
    }

    SECTION("When we are logged in and time is not expired") {
        auth.expiry_time = make_time_([](time_t t) { return t + 60; });
        auth.logged_in = true;
        auth.auth_token = "{TOKEN}";
        REQUIRE(auth.is_valid(Logger::global()) == true);
    }
}

TEST_CASE("orchestrate::login() works correctly") {
    auto reactor = Reactor::make();

    SECTION("When the username is missing") {
        Error err;
        reactor->run_with_initial_event([&]() {
            login({}, testing_registry_url(), {}, reactor, Logger::global(),
                  [&](Error &&e, Auth &&) {
                      err = e;
                      reactor->stop();
                  });
        });
        REQUIRE(err == MissingRequiredValueError());
    }

    SECTION("When the password is missing") {
        Auth auth;
        auth.username = "antani";
        Error err;
        reactor->run_with_initial_event([&]() {
            login(std::move(auth), testing_registry_url(), {}, reactor,
                  Logger::global(), [&](Error &&e, Auth &&) {
                      err = e;
                      reactor->stop();
                  });
        });
        REQUIRE(err == MissingRequiredValueError());
    }

    /*
     * TODO: add more tests
     */
}

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("Orchestration works") {
    Client client;
    client.logger->increase_verbosity();
    client.geoip_country_path = "GeoIP.dat";
    client.geoip_asn_path = "GeoIPASNum.dat";
    client.network_type = "wifi";
    // client.device_token = "{TOKEN}";  /* Not needed on PC devices */
    client.registry_url = testing_registry_url();
    std::promise<Error> promise;
    std::future<Error> future = promise.get_future();
    client.register_probe("", [&promise, &client](Error &&error, Auth &&auth) {
        if (error) {
            promise.set_value(error);
            return;
        }
        // This is what you should do right after you are registered: you
        // should dump the `Auth` structure on persistent storage
        auto path = []() {
            std::string s = "orchestrator_secrets_";
            s += random_str(8);
            s += ".json";
            return s;
        }();
        if ((error = auth.dump(path)) != NoError()) {
            promise.set_value(error);
            return;
        }
        // In theory you should not call `update` immediately so here we
        // modify network type to simulate a change. We also reload the auth
        // as this is typically what you would do after some time.
        client.network_type = "3g";
        auto saved_username = auth.username;
        auto saved_password = auth.password;
        auth = {}; // clear
        if ((error = auth.load(path)) != NoError()) {
            promise.set_value(error);
            return;
        }
        client.update(std::move(auth), [&promise, &client](Error &&error,
                                                           Auth &&auth) {
            if (error) {
                promise.set_value(error);
                return;
            }
            // Do it twice to see if the token is still valid
            client.update(std::move(auth), [&promise](Error &&error, Auth &&) {
                promise.set_value(error);
            });
        });
    });
    REQUIRE(future.get() == NoError());
    /*
     * Wait for the default tasks queue to empty, so we exit from the
     * process without still running detached threads and we don't leak
     * memory and, therefore, valgrind memcheck does not fail.
     *
     * See also `test/nettests/utils.hpp`.
     */
    Worker::default_tasks_queue()->wait_empty_();
}

#endif
