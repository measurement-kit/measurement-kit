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

using chai_t = chaiscript::ChaiScript;
using chai_generic_t = chaiscript::Boxed_Value;
using chai_map_t = std::map<std::string, chai_generic_t>;
using chai_vector_t = std::vector<chai_generic_t>;

struct YieldFrom {
    chai_t &chai;
    std::atomic<size_t> complete{0};
    std::vector<Callback<>> finalizers;
    Var<Reactor> reactor = Reactor::make();
    chai_vector_t retval;
    std::vector<Callback<>> runners;

    YieldFrom(chai_t &owner) : chai(owner) {}
};

template <typename T>
void maybe_copy_from_chai(Var<YieldFrom> YF, Settings &s, const chai_map_t &m,
                          std::string k) {
    if (m.find(k) != m.end()) {
        s[k] = YF->chai.boxed_cast<T>(m.at(k));
    }
}

static void maybe_stop_reactor(Var<YieldFrom> YF) {
    if (++YF->complete >= YF->retval.size()) {
        YF->reactor->break_loop();
    }
}

static Callback<> eval_dns_query(Var<YieldFrom> YF, size_t index,
                                 chai_vector_t expression) {
    auto qclass = YF->chai.boxed_cast<std::string>(expression.at(1));
    auto qtype = YF->chai.boxed_cast<std::string>(expression.at(2));
    auto qquery = YF->chai.boxed_cast<std::string>(expression.at(3));
    auto qmap = YF->chai.boxed_cast<chai_map_t>(expression.at(4));
    Settings settings;
    maybe_copy_from_chai<std::string>(YF, settings, qmap, "dns/nameserver");
    maybe_copy_from_chai<int>(YF, settings, qmap, "dns/attempts");
    maybe_copy_from_chai<double>(YF, settings, qmap, "dns/timeout");
    return [=]() {
        dns::query(qclass.c_str(), qtype.c_str(), qquery,
                   [=](Error err, dns::Message message) {
                       maybe_stop_reactor(YF);
                       YF->finalizers.at(index) = [=]() {
                           chai_vector_t tuple;
                           tuple.push_back(chai_generic_t{err});
                           tuple.push_back(chai_generic_t{message});
                           YF->retval.at(index) = chai_generic_t{tuple};
                       };
                   },
                   settings, YF->reactor);
    };
}

static Callback<> do_eval(Var<YieldFrom> YF, size_t index, std::string form,
                          chai_vector_t expression) {
    if (form == "dns_query") {
        return eval_dns_query(YF, index, expression);
    }
    // TODO: expose more MK APIs
    throw std::runtime_error("asked to evaluate an unknown form");
}

static chai_vector_t do_yield_from(chai_t &chai, chai_vector_t args) {
    Var<YieldFrom> YF{new YieldFrom{chai}};
    YF->retval.resize(args.size()); /* YF->retval.size() used to stop reactor */
    YF->finalizers.resize(args.size());
    YF->runners.resize(args.size());
    size_t index = 0;
    for (auto arg : args) {
        auto expression = YF->chai.boxed_cast<chai_vector_t>(arg);
        auto form = YF->chai.boxed_cast<std::string>(expression.at(0));
        YF->runners.at(index) = do_eval(YF, index, form, expression);
        index += 1;
    }
    YF->reactor->loop_with_initial_event([=]() {
        for (auto f : YF->runners) {
            f();
        }
    });
    for (auto f : YF->finalizers) {
        f();
    }
    return YF->retval;
}

Error chai(std::string filepath) {
    chai_t chai(chaiscript::Std_Lib::library());

    /*
     * expose common
     */

    chai.add(chaiscript::user_type<Error>(), "Error");
    chai.add(chaiscript::fun(&Error::code), "code");
    chai.add(chaiscript::fun(&Error::reason), "reason");

    /*
     * expose dns
     */

    chai.add(chaiscript::user_type<dns::Message>(), "DnsMessage");
    chai.add(chaiscript::fun(&dns::Message::rtt), "rtt");
    chai.add(chaiscript::fun(&dns::Message::error_code), "error_code");
    chai.add(chaiscript::fun(&dns::Message::queries), "queries");
    chai.add(chaiscript::fun(&dns::Message::answers), "answers");

    chai.add(chaiscript::bootstrap::standard_library::vector_type<
             std::vector<dns::Query>>("DnsQueryList"));
    chai.add(chaiscript::bootstrap::standard_library::vector_type<
             std::vector<dns::Answer>>("DnsAnswerList"));

    chai.add(chaiscript::user_type<dns::Query>(), "DnsQuery");
    chai.add(chaiscript::fun(&dns::Query::type), "type");
    chai.add(chaiscript::fun(&dns::Query::qclass), "qclass");
    chai.add(chaiscript::fun(&dns::Query::ttl), "ttl");
    chai.add(chaiscript::fun(&dns::Query::name), "name");

    chai.add(chaiscript::user_type<dns::Answer>(), "DnsAnswer");
    chai.add(chaiscript::fun(&dns::Answer::type), "type");
    chai.add(chaiscript::fun(&dns::Answer::qclass), "qclass");
    chai.add(chaiscript::fun(&dns::Answer::code), "code");
    chai.add(chaiscript::fun(&dns::Answer::ttl), "ttl");
    chai.add(chaiscript::fun(&dns::Answer::ipv4), "ipv4");
    chai.add(chaiscript::fun(&dns::Answer::ipv6), "ipv6");
    chai.add(chaiscript::fun(&dns::Answer::hostname), "hostname");

    chai.add(chaiscript::user_type<dns::QueryClass>(), "DnsQueryClass");
    chai.add(chaiscript::fun(&dns::QueryClass::str), "str");

    chai.add(chaiscript::user_type<dns::QueryType>(), "DnsQueryType");
    chai.add(chaiscript::fun(&dns::QueryType::str), "str");

    /*
     * declare our yield-like operator and run
     */

    chai.add(chaiscript::fun(
                 [&](chai_vector_t args) { return do_yield_from(chai, args); }),
             "yield_from");

    chai.eval_file(filepath);
    return NoError();
}

} // namespace scripting
} // namespace mk
