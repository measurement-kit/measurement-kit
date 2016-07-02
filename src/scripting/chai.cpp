// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <atomic>
#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/scripting.hpp>

namespace mk {
namespace scripting {

using Any = chaiscript::Boxed_Value;
using List = std::vector<Any>;
using Str = std::string;
using Dict = std::map<Str, Any>;

#define to_list(x) chaiscript::boxed_cast<List>(x)
#define to_str(x) chaiscript::boxed_cast<Str>(x)
#define to_dict(x) chaiscript::boxed_cast<Dict>(x)
#define to_double(x) chaiscript::boxed_cast<double>(x)
#define to_int(x) chaiscript::boxed_cast<int>(x)

struct YieldFrom {
    std::atomic<size_t> complete{0};
    std::list<Callback<>> input;
    std::vector<List> output;
    Var<Reactor> reactor = Reactor::make();
};

static void yield_from(Any args) {
    Var<YieldFrom> Y(new YieldFrom);
    auto coros = to_list(args);
    int index = 0;
    for (auto entry : coros) {
        auto coro = to_list(entry);
        auto operation = to_str(coro[0]);
        if (operation == "dns_query") {
            warn("here1");
            auto qclass = to_str(coro[1]);
            auto qtype = to_str(coro[2]);
            auto qquery = to_str(coro[3]);
            warn("here2");
            auto qs = to_dict(coro[4]);
            Settings settings;
            for (auto pair : qs) {
                auto key = pair.first;
                if (key == "dns/timeout") {
                    settings[key] = to_double(pair.second);
                } else if (key == "dns/attempts") {
                    settings[key] = to_int(pair.second);
                } else {
                    settings[key] = to_str(pair.second);
                }
            }
            warn("here3");
            Y->input.push_back([=]() {
                dns::query(qclass.c_str(), qtype.c_str(), qquery,
                           [=](Error err, dns::Message msg) {
                               if (++Y->complete >= Y->input.size()) {
                                   Y->reactor->break_loop();
                               }
                               // TODO: fill response
                           },
                           settings, Y->reactor);
            });
            warn("here4");
        } else {
            Y->input.push_back([]() {});
        }
        ++index;
        //Y->output[index] = {};
        warn("here5");
    }
    warn("here6");
    Y->reactor->loop_with_initial_event([=]() {
        for (auto func : Y->input) {
            func();
        }
    });
    warn("here7");
    //return chaiscript::boxed_cast<Any>(Y->output);
}

Error chai(std::string filepath) {
    chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());
    chai.add(chaiscript::fun(&yield_from), "yield_from");
    chai.eval_file(filepath);
    return NoError();
}

} // namespace scripting
} // namespace mk
