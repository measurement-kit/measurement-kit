// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include "../ooni/constants.hpp"
#include "../ooni/utils.hpp"

namespace mk {
namespace ooni {

using namespace mk::report;

auto status_code_correct = [=](Var<http::Response> response, std::string expected) {
    return response->status_code == std::stoi(expected); // this might be avoidable
};

auto body_correct = [=](Var<http::Response> response, std::string expected) {
    return response->body == expected;
};

typedef std::map<std::string,std::string> input;
void gen_http_inputs(Var<std::vector<input>> is, Var<Logger> logger) {
    logger->info("starting to gen");
    input i;
    i["name"] = "Microsoft HTTP";
    i["url"] = "http://www.msftncsi.com/ncsi.txt";
    i["body"] = "Microsoft NCSI";
    is->push_back(i);
    i.clear();
    i["name"] = "Apple HTTP 1";
    i["url"] = "http://captive.apple.com";
    i["body"] = "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>\n";
    i["ua"] = "CaptiveNetworkSupport/1.0 wispr";
    is->push_back(i);
    i.clear();
    i["name"] = "Apple HTTP 2";
    i["url"] = "http://captive.apple.com/hotspot_detect.html";
    i["body"] = "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>\n";
    i["ua"] = "CaptiveNetworkSupport/1.0 wispr";
    is->push_back(i);
    i.clear();
    i["name"] = "Android KitKat HTTP";
    i["url"] = "http://clients3.google.com/generate_204";
    i["status"] = "204"; //XXX: fml
    is->push_back(i);
    i.clear();
    i["name"] = "Android Lollipop HTTP";
    i["url"] = "http://connectivitycheck.android.com/generate_204";
    i["status"] = "204";
    is->push_back(i);
    i.clear();
    i["name"] = "Android Marshmallow HTTP";
    i["url"] = "http://connectivitycheck.gstatic.com/generate_204";
    i["status"] = "204";
    is->push_back(i);
    i.clear();
    i["name"] = "Android Nougat HTTP 1";
    i["url"] = "https://www.google.com/generate_204";
    i["status"] = "204";
    i["ua"] = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36";
    is->push_back(i);
    i.clear();
    i["name"] = "Android Nougat HTTP 2";
    i["url"] = "http://www.google.com/gen_204";
    i["status"] = "204";
    i["ua"] = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36";
    is->push_back(i);
    i.clear();
    i["name"] = "Android Nougat HTTP 3";
    i["url"] = "http://play.googleapis.com/generate_204";
    i["status"] = "204";
    i["ua"] = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36";
    is->push_back(i);
    i.clear();
}

// TODO: rename input -> input_t
typedef std::function<bool(Var<http::Response>)> no_cp_f_t;
void gen_no_cp_f(const input &i, Var<no_cp_f_t> no_cp_f) {
    if (i.count("body") && i.count("status")) {
        (*no_cp_f) = [=](Var<http::Response> r) {
            return r->body == i.at("body") &&
                r->status_code == std::stoi(i.at("status"));
        };
    } else if (i.count("body")) {
        (*no_cp_f) = [=](Var<http::Response> r) {
            return r->body == i.at("body");
        };
    } else if (i.count("status")) {
        (*no_cp_f) = [=](Var<http::Response> r) {
            return r->status_code == std::stoi(i.at("status"));
        };
    } else {
        exit(1); // bug
    }
}

void http_many(Var<Entry> entry,
               Callback<Error> all_done_cb,
               Var<Reactor> reactor,
               Var<Logger> logger) {

    auto http_cb = [=](std::string name, Var<no_cp_f_t> no_cp_f, Callback<Error> done_cb) {
        return [=](Error err, Var<http::Response> response) {
            if (!!err) {
                logger->info("err: %s", err.as_ooni_error().c_str());
                done_cb(err);
            } else if (!response) {
                logger->info("null response");
                done_cb(NoError());
            } else {
                bool no_cp = (*no_cp_f)(response);
                if (no_cp) {
                    logger->info("no captive portal at %s", name.c_str());
                } else {
                    logger->info("yes captive portal at %s", name.c_str());
                }
                done_cb(NoError());
            }
        };
    };

    Var<std::vector<input>> inputs(new std::vector<input>);
    gen_http_inputs(inputs, logger);

    std::vector<Continuation<Error>> continuations;
    for (auto input : *inputs) {
        logger->info("setting up %s", input.at("name").c_str());
        Settings http_options; //XXX: specify timeout here
        http_options["http/url"] = input["url"];
        std::string body;
        http::Headers headers;
        if (input.count("ua")) {
            headers = { { "User-Agent", input.at("ua") } };
        } else {
            headers = constants::COMMON_CLIENT_HEADERS;
        }

        Var<no_cp_f_t> no_cp_f(new no_cp_f_t);
        gen_no_cp_f(input, no_cp_f);

        continuations.push_back(
            [=](Callback<Error> done_cb) {
                templates::http_request(entry, http_options, headers, body,
                    http_cb(input.at("name"), no_cp_f, done_cb), reactor, logger);
            }
        );
    }
    mk::parallel(continuations, all_done_cb, 3);
}

void dns_msft_ncsi(Var<Entry> entry,
                   Callback<Error> done_cb,
                   Var<Reactor> reactor,
                   Var<Logger> logger) {
    Settings options;
    std::string hostname = "dns.msftncsi.com";
    std::string nameserver = "";
    templates::dns_query(
        entry, "A", "IN", hostname, nameserver,
        [=](Error err, Var<dns::Message> message) {
            if (!!err) {
                logger->info("dns_query err: %s", err.as_ooni_error().c_str());
                done_cb(err);
            } else {
                if (message->answers.size() != 1) {
                    logger->info("maybe captive portal");
                } else if (message->answers[0].ipv4 != "131.107.255.255") {
                    logger->info("probably captive portal");
                } else {
                    logger->info("no captive portal");
                }
                done_cb(NoError());
            }
        }, options, reactor, logger);
}

void captive_portal(std::string input, Settings options,
                    Callback<Var<Entry>> callback, Var<Reactor> reactor,
                    Var<Logger> logger) {
    Var<Entry> entry(new Entry);

    logger->info("starting http_many");
    http_many(entry, [=](Error err){
        if (!!err) {
            logger->info("http_many error");
        }
        logger->info("starting dns_msft_ncsi");
        dns_msft_ncsi(entry, [=](Error err){
            if (!!err) {
                logger->info("dns_msft_ncsi error");
            }
            callback(entry);
        }, reactor, logger);
    }, reactor, logger);


}

} // namespace ooni
} // namespace mk
