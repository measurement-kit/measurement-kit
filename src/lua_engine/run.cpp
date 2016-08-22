// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <atomic>
#include <lua.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/lua_engine.hpp>
#include <vector>

extern "C" {
using namespace mk;

static void require_num_arguments(lua_State *L, int n) {
    if (lua_gettop(L) != n) {
        warn("invalid number of arguments");
        throw GenericError();
    }
}

static lua_State *get_thread(lua_State *L, int n) {
    if (lua_type(L, n) != LUA_TTHREAD) {
        warn("expected a coroutine argument");
        throw GenericError();
    }
    return lua_tothread(L, n);
}

static std::string get_string(lua_State *L, int n) {
    if (lua_type(L, n) != LUA_TSTRING) {
        warn("expected a string argument");
        throw GenericError();
    }
    return lua_tostring(L, n);
}

static Settings get_settings(lua_State *L, int n) {
    Settings settings;
    if (lua_type(L, n) != LUA_TTABLE) {
        warn("expected a table argument");
        throw GenericError();
    }
    for (lua_pushnil(L); lua_next(L, n) != 0; lua_pop(L, 1)) {
        if (lua_type(L, -2) != LUA_TSTRING) {
            warn("expected a string key");
            throw GenericError();
        }
        int value_type = lua_type(L, -1);
        if (value_type != LUA_TSTRING and value_type != LUA_TNUMBER) {
            warn("expected value convertible to string");
            throw GenericError();
        }
        settings[lua_tostring(L, -2)] = lua_tostring(L, -1);
    }
    return settings;
}

static void push_error(lua_State *coro, const Error &err) {
    lua_newtable(coro);

    lua_pushstring(coro, "code");
    lua_pushinteger(coro, err.code);
    lua_settable(coro, -3);

    lua_pushstring(coro, "reason");
    lua_pushstring(coro, err.reason.c_str());
    lua_settable(coro, -3);
}

static void push_dns_answer_(lua_State *coro, const dns::Answer &answer) {

    lua_pushstring(coro, "query_type");
    if (answer.type == dns::QueryTypeId::A) {
        lua_pushstring(coro, "A");
    } else if (answer.type == dns::QueryTypeId::AAAA) {
        lua_pushstring(coro, "AAAA");
    } else if (answer.type == dns::QueryTypeId::PTR) {
        lua_pushstring(coro, "PTR");
    } else {
        lua_pushnil(coro);
    }
    lua_settable(coro, -3);

    lua_pushstring(coro, "query_class");
    if (answer.qclass == dns::QueryClassId::IN) {
        lua_pushstring(coro, "IN");
    } else {
        lua_pushnil(coro);
    }
    lua_settable(coro, -3);

    lua_pushstring(coro, "code");
    lua_pushinteger(coro, answer.code);
    lua_settable(coro, -3);

    lua_pushstring(coro, "ttl");
    lua_pushinteger(coro, answer.ttl);
    lua_settable(coro, -3);

    lua_pushstring(coro, "name");
    lua_pushstring(coro, answer.name.c_str());
    lua_settable(coro, -3);

    lua_pushstring(coro, "ipv4");
    lua_pushstring(coro, answer.ipv4.c_str());
    lua_settable(coro, -3);

    lua_pushstring(coro, "ipv6");
    lua_pushstring(coro, answer.ipv6.c_str());
    lua_settable(coro, -3);

    lua_pushstring(coro, "hostname");
    lua_pushstring(coro, answer.hostname.c_str());
    lua_settable(coro, -3);
}

static void push_dns_message(lua_State *coro, const dns::Message &msg) {
    lua_newtable(coro);

    lua_pushstring(coro, "rtt");
    lua_pushnumber(coro, msg.rtt);
    lua_settable(coro, -3);

    lua_pushstring(coro, "error_code");
    lua_pushinteger(coro, msg.error_code);
    lua_settable(coro, -3);

    lua_pushstring(coro, "answers");
    lua_newtable(coro);
    for (size_t i = 0; i < msg.answers.size(); ++i) {
        lua_pushinteger(coro, i);
        lua_newtable(coro);
        push_dns_answer_(coro, msg.answers[i]);
        lua_settable(coro, -3);
    }
    lua_settable(coro, -3);
 
    lua_pushstring(coro, "queries");
    lua_newtable(coro);
    for (size_t i = 0; i < msg.queries.size(); ++i) {
        //lua_rawseti(coro, -2, i);
    }
    lua_settable(coro, -3);
}

static void do_resume_two(lua_State *L, lua_State *coro) {
    int error = lua_resume(coro, L, 2);
    if (error != LUA_YIELD) {
        warn("unexpected lua_resume() return value");
        throw GenericError();
    }
}

static int _dns_query(lua_State *L) {
    require_num_arguments(L, 5);
    lua_State *coro = get_thread(L, 1);
    std::string qclass = get_string(L, 2);
    std::string qtype = get_string(L, 3);
    std::string name = get_string(L, 4);
    Settings settings = get_settings(L, 5);
    call_later(1.0, [=]() {
        dns::query(qclass.c_str(), qtype.c_str(), name,
                   [=](Error err, dns::Message msg) {
            debug("dns::query() complete");
            push_dns_message(coro, msg);
            push_error(coro, err);
            do_resume_two(L, coro);
        }, settings);
        debug("dns::query() pending");
    });
    debug("registered delayed dns::query()");
    return 0;
}

static int _loop_once(lua_State *) {
    loop_once();
    return 0;
}

} // extern "C"
namespace mk {
namespace lua_engine {

static Var<lua_State> make_lua_state() {
    Var<lua_State> state(luaL_newstate(), [](lua_State *ptr) {
        if (ptr != nullptr) {
            lua_close(ptr);
        }
    });
    if (!state) {
        throw std::bad_alloc();
    }
    return state;
}

static void run_script(lua_State *L, std::string path) {
    debug("running %s", path.c_str());
    int error = luaL_loadfile(L, path.c_str());
    if (error) {
        throw GenericError();
    }
    error = lua_pcall(L, 0, 0, 0);
    if (error) {
        warn("lua_pcall failed");
        throw GenericError();
    }
}

static void set_mk_func(lua_State *L, std::string n, int (*f)(lua_State *)) {
    lua_getglobal(L, "mk");
    lua_pushstring(L, n.c_str());
    lua_pushcfunction(L, f);
    lua_settable(L, -3);
}

void run(std::string path) {
    Var<lua_State> state = make_lua_state();
    auto L = state.get();
    luaL_openlibs(L);
    run_script(L, "usr/lib/measurement_kit/lua_engine/_runtime_begin.lua");

    set_mk_func(L, "_dns_query", _dns_query);
    set_mk_func(L, "_loop_once", _loop_once);

    run_script(L, path);
    run_script(L, "usr/lib/measurement_kit/lua_engine/_runtime_end.lua");
}

} // namespace lua_engine
} // namespace mk
