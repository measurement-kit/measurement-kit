// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/common/utils.hpp"
#include <atomic>
#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/scripting.hpp>

namespace mk {
namespace scripting {

using chai_generic_t = chaiscript::Boxed_Value;
using chai_map_t = std::map<std::string, chai_generic_t>;
using chai_vector_t = std::vector<chai_generic_t>;

struct Resume {
    std::atomic<size_t> complete{0};
    std::vector<Callback<>> finalizers;
    Var<Reactor> reactor = Reactor::make();
    chai_vector_t retval;
    std::vector<Callback<>> runners;
};

template <typename T>
void maybe_copy_from_chai(Settings &s, const chai_map_t &m, std::string k) {
    if (m.find(k) != m.end()) {
        s[k] = chaiscript::boxed_cast<T>(m.at(k));
    }
}

static void maybe_stop_reactor(Var<Resume> R) {
    if (++R->complete >= R->retval.size()) {
        R->reactor->break_loop();
    }
}

static Callback<> eval_dns_query(Var<Resume> R, size_t index,
                                 chai_vector_t expression) {
    auto qclass = chaiscript::boxed_cast<std::string>(expression.at(1));
    auto qtype = chaiscript::boxed_cast<std::string>(expression.at(2));
    auto qquery = chaiscript::boxed_cast<std::string>(expression.at(3));
    auto qmap = chaiscript::boxed_cast<chai_map_t>(expression.at(4));
    Settings settings;
    maybe_copy_from_chai<std::string>(settings, qmap, "dns/nameserver");
    maybe_copy_from_chai<int>(settings, qmap, "dns/attempts");
    maybe_copy_from_chai<double>(settings, qmap, "dns/timeout");
    return [=]() {
        dns::query(qclass.c_str(), qtype.c_str(), qquery,
                   [=](Error err, dns::Message msg) {
                       maybe_stop_reactor(R);
                       R->finalizers.at(index) = [=]() {
                           chai_vector_t tuple;
                           tuple.push_back(chai_generic_t{err.as_ooni_error()});
                           chai_vector_t message;
                           message.push_back(
                               chai_generic_t{std::string{"dns_message"}});
                           message.push_back(chai_generic_t{msg.rtt});
                           message.push_back(chai_generic_t{msg.error_code});
                           chai_vector_t queries;
                           queries.push_back(
                               chai_generic_t{std::string{"queries"}});
                           for (auto q: msg.queries) {
                               chai_vector_t cq;
                               cq.push_back(chai_generic_t{q.type.str()});
                               cq.push_back(chai_generic_t{q.qclass.str()});
                               cq.push_back(chai_generic_t{q.ttl});
                               cq.push_back(chai_generic_t{q.name});
                               queries.push_back(chai_generic_t{cq});
                           }
                           message.push_back(chai_generic_t{queries});
                           chai_vector_t answers;
                           answers.push_back(
                               chai_generic_t{std::string{"answers"}});
                           for (auto a: msg.answers) {
                               chai_vector_t ca;
                               ca.push_back(chai_generic_t{a.type.str()});
                               ca.push_back(chai_generic_t{a.qclass.str()});
                               ca.push_back(chai_generic_t{a.code});
                               ca.push_back(chai_generic_t{a.ttl});
                               //ca.push_back(chai_generic_t{a.name}); // ???
                               if (a.type == dns::QueryTypeId::A) {
                                   ca.push_back(chai_generic_t{a.ipv4});
                               } else if (a.type == dns::QueryTypeId::AAAA) {
                                   ca.push_back(chai_generic_t{a.ipv6});
                               } else if (a.type == dns::QueryTypeId::PTR) {
                                   ca.push_back(chai_generic_t{a.hostname});
                               } else {
                                   /* TODO */ ;
                               }
                               answers.push_back(chai_generic_t{ca});
                           }
                           message.push_back(chai_generic_t{answers});
                           tuple.push_back(chai_generic_t{message});
                           R->retval.at(index) = chai_generic_t{tuple};
                       };
                   },
                   settings, R->reactor);
    };
}

static Callback<> do_eval(Var<Resume> R, size_t index, std::string form,
                          chai_vector_t expression) {
    if (form == "dns_query") {
        return eval_dns_query(R, index, expression);
    }
    // TODO: expose more MK APIs
    throw std::runtime_error("asked to evaluate an unknown form");
}

static chai_vector_t do_yield_from(chai_vector_t args) {
    Var<Resume> R{new Resume};
    R->retval.resize(args.size()); /* R->retval.size() used to stop reactor */
    R->finalizers.resize(args.size());
    R->runners.resize(args.size());
    size_t index = 0;
    for (auto arg : args) {
        auto expression = chaiscript::boxed_cast<chai_vector_t>(arg);
        auto form = chaiscript::boxed_cast<std::string>(expression.at(0));
        R->runners.at(index) = do_eval(R, index, form, expression);
        index += 1;
    }
    R->reactor->loop_with_initial_event([=]() {
        for (auto f : R->runners) {
            f();
        }
    });
    for (auto f : R->finalizers) {
        f();
    }
    return R->retval;
}

Error chai(std::string filepath) {
    chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());
    chai.add(chaiscript::fun(&do_yield_from), "yield_from");
    chai.eval_file(filepath);
    return NoError();
}

} // namespace scripting
} // namespace mk
