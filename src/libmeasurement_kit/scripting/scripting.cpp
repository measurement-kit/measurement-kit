// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define SOL_CHECK_ARGUMENTS

#include <measurement_kit/http.hpp>
#include <measurement_kit/scripting.hpp>

#include "../ext/sol.hpp"

namespace mk {
namespace scripting {

void run(std::string script, Var<Reactor> reactor, Var<Logger> logger) {

    // Implementation note: this function is blocking and we are not going to
    // leave it until both the I/O thread and the foreground lua thread have
    // exited, therefore we can safely use `&` to capture lambdas context.

    logger->debug("Running script: %s", script.c_str());

    sol::state state;
    state.open_libraries(sol::lib::base, sol::lib::math);
    std::atomic<int> running{0};

    // Declare API functions:

    state.set_function("http_request", [&](sol::table settings,
                                           sol::table headers, sol::object body,
                                           sol::function callback) {
        Settings cxx_settings;
        settings.for_each([&](sol::object key, sol::object value) {
            cxx_settings[key.as<std::string>()] = value.as<std::string>();
        });
        http::Headers cxx_headers;
        headers.for_each([&](sol::object key, sol::object value) {
            cxx_headers[key.as<std::string>()] = value.as<std::string>();
        });
        running += 1;
        // Note: here we MUST capture `callback` by value otherwise `sol2`
        // would not keep it safe while we're doing I/O
        http::request(cxx_settings, cxx_headers, body.as<std::string>(),
                      [callback, &reactor,
                       &running, &state](Error error, Var<http::Response> response) {
                          if (--running <= 0) {
                              reactor->break_loop();
                          }
                          sol::table lua_error = state.create_table();
                          lua_error["code"] = error.code;
                          lua_error["reason"] = error.reason;
                          sol::table lua_response = state.create_table();
                          lua_response["status_code"] = response->status_code;
                          lua_response["reason"] = response->reason;
                          callback(lua_error, lua_response);
                      });
    });

    state.script_file(script);
    reactor->loop();
}

} // namespace scripting
} // namespace mk
