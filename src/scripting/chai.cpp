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

struct Resume {
    chai_t &chai;
    std::atomic<size_t> complete{0};
    std::vector<Callback<>> finalizers;
    Var<Reactor> reactor = Reactor::make();
    chai_vector_t retval;
    std::vector<Callback<>> runners;

    Resume(chai_t &owner) : chai(owner) {}
};

template <typename T>
void maybe_copy_from_chai(Var<Resume> R, Settings &s, const chai_map_t &m,
                          std::string k) {
    if (m.find(k) != m.end()) {
        s[k] = R->chai.boxed_cast<T>(m.at(k));
    }
}

static void maybe_stop_reactor(Var<Resume> R) {
    if (++R->complete >= R->retval.size()) {
        R->reactor->break_loop();
    }
}

static Callback<> eval_dns_query(Var<Resume> R, size_t index,
                                 chai_vector_t expression) {
    auto qclass = R->chai.boxed_cast<std::string>(expression.at(1));
    auto qtype = R->chai.boxed_cast<std::string>(expression.at(2));
    auto qquery = R->chai.boxed_cast<std::string>(expression.at(3));
    auto qmap = R->chai.boxed_cast<chai_map_t>(expression.at(4));
    Settings settings;
    maybe_copy_from_chai<std::string>(R, settings, qmap, "dns/nameserver");
    maybe_copy_from_chai<int>(R, settings, qmap, "dns/attempts");
    maybe_copy_from_chai<double>(R, settings, qmap, "dns/timeout");
    return [=]() {
        dns::query(qclass.c_str(), qtype.c_str(), qquery,
                   [=](Error err, dns::Message message) {
                       maybe_stop_reactor(R);
                       R->finalizers.at(index) = [=]() {
                           chai_vector_t tuple;
                           tuple.push_back(chai_generic_t{err});
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

static chai_vector_t do_yield_from(chai_t &chai, chai_vector_t args) {
    Var<Resume> R{new Resume{chai}};
    R->retval.resize(args.size()); /* R->retval.size() used to stop reactor */
    R->finalizers.resize(args.size());
    R->runners.resize(args.size());
    size_t index = 0;
    for (auto arg : args) {
        auto expression = R->chai.boxed_cast<chai_vector_t>(arg);
        auto form = R->chai.boxed_cast<std::string>(expression.at(0));
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
    chai_t chai(chaiscript::Std_Lib::library());
    chai.add(chaiscript::fun(&do_yield_from), "yield_from");
    chai.add(chaiscript::fun(
                 [&](chai_vector_t args) { return do_yield_from(chai, args); }),
             "yield_from");
    chai.add(chaiscript::fun([]() { return Error(); }), "xo");

    chai.add(chaiscript::user_type<Error>(), "Error");
    chai.add(chaiscript::fun(&Error::code), "code");
    chai.add(chaiscript::fun(&Error::reason), "reason");

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

    chai.eval_file(filepath);
    return NoError();
}

} // namespace scripting
} // namespace mk
