// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <atomic>
#include <lua.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/script.hpp>
#include <vector>

extern "C" {
    using namespace mk;

struct YieldContext {
    std::atomic<size_t> complete{0};
    Var<Reactor> reactor = Reactor::make();
    std::vector<Callback<>> todo_list;
};

static int mk_yield(lua_State *L) {
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "mk_yield: expected one argument");
    }
    if (lua_type(L, 1) != LUA_TTABLE) {
        return luaL_error(L, "mk_yield: expected a TABLE argument");
    }

    Var<YieldContext> YC(new YieldContext);

    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        if (lua_type(L, -2) != LUA_TNUMBER) {
            return luaL_error(L, "mk_yield: expected a NUMBER key");
        }
        int index = lua_tointeger(L, -2);

        if (lua_type(L, -1) != LUA_TTABLE) {
            return luaL_error(L, "mk_yield: expected a TABLE value");
        }

        if (lua_getfield(L, -1, "operation") != LUA_TSTRING) {
            return luaL_error(L, "mk_yield: operation must be a STRING");
        }
        std::string operation = lua_tostring(L, -1);
        lua_pop(L, 1);

        if (operation == "dns_query") {
            if (lua_getfield(L, -1, "dns_class") != LUA_TSTRING) {
                return luaL_error(L, "mk_yield: dns_class must be a STRING");
            }
            std::string dns_class = lua_tostring(L, -1);
            lua_pop(L, 1);

            if (lua_getfield(L, -1, "dns_type") != LUA_TSTRING) {
                return luaL_error(L, "mk_yield: dns_type must be a STRING");
            }
            std::string dns_type = lua_tostring(L, -1);
            lua_pop(L, 1);

            if (lua_getfield(L, -1, "query") != LUA_TSTRING) {
                return luaL_error(L, "mk_yield: query must be a STRING");
            }
            std::string query = lua_tostring(L, -1);
            lua_pop(L, 1);

            if (lua_getfield(L, -1, "nameserver") != LUA_TSTRING) {
                return luaL_error(L, "mk_yield: nameserver must be a STRING");
            }
            std::string nameserver = lua_tostring(L, -1);
            lua_pop(L, 1);

            YC->todo_list.push_back([=]() {
                dns::query(dns_class.c_str(), dns_type.c_str(), query,
                           [=](Error err, dns::Message msg) {
                               // Note: we assume to have on top of the
                               // stack the table used as return value
                               lua_pushinteger(L, index);
                               lua_newtable(L);
                               lua_pushstring(L, "error_code");
                               lua_pushinteger(L, err.code);
                               lua_settable(L, -3);
                               lua_settable(L, -3);
                               if (++YC->complete >= YC->todo_list.size()) {
                                   YC->reactor->break_loop();
                               }
                           },
                           {{"dns/nameserver", nameserver},
                            {"dns/attempts", 1},
                            {"dns/timeout", 0.5}},
                           YC->reactor);
            });

        } else {
            return luaL_error(L, "mk_yield: unknown operation: %s",
                              operation.c_str());
        }

        lua_pop(L, 1); // Remove value from top of the stack
    }
    lua_pop(L, 1); // Remove table from top of the stack

    if (YC->todo_list.size() <= 0) {
        return 0;
    }
    lua_newtable(L);
    YC->reactor->loop_with_initial_event([=]() {
        for (auto fire : YC->todo_list) {
            fire();
        }
    });

    return 1;
}

} // extern "C"
namespace mk {
namespace script {

static Var<lua_State> make_lua_state() {
    return Var<lua_State>(luaL_newstate(), [](lua_State *ptr) {
        if (ptr != nullptr) {
            lua_close(ptr);
        }
    });
}

Error run_lua(std::string path) {
    std::vector<Settings> pending_ops;
    Var<lua_State> state = make_lua_state();
    if (!state) {
        return GenericError();
    }
    /*
    auto ppending = (std::vector<Settings> **)lua_getextraspace(state.get());
    *ppending = &pending_ops;
    */
    luaL_openlibs(state.get());
    lua_register(state.get(), "mk_yield", mk_yield);
    int error = luaL_loadfile(state.get(), path.c_str());
    if (error) {
        return GenericError();
    }
    error = lua_pcall(state.get(), 0, 0, 0);
    if (error) {
        const char *reason = lua_tostring(state.get(), -1);
        if (reason) {
            warn("lua_pcall: %s", reason);
        }
        return GenericError();
    }
    return NoError();
}

} // namespace script
} // namespace mk
