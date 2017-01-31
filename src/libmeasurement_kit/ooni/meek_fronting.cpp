// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

void meek_fronting(Settings options,
                               Callback<Var<report::Entry>> cb,
                               Var<Reactor> reactor, Var<Logger> logger) {
//    Var<report::Entry> entry(new report::Entry);
//    (*entry)["tampering"] = nullptr;
//    (*entry)["received"] = report::Entry::array();
//    (*entry)["sent"] = report::Entry::array();
//    Var<int> tests_run(new int(0));
//
//    ErrorOr<http::Url> backend_url =
//        mk::http::parse_url_noexcept(options["backend"]);
//
//    if (!backend_url) {
//        logger->debug("Invalid backend url.");
//        cb(entry);
//        return;
//    }
//
//    auto handle_response = [=]() {
//        *tests_run += 1;
//        if (*tests_run == 4) {
//            cb(entry);
//        }
//    };
//
//    // test_random_invalid_method
//    // randomSTR(4) + " / HTTP/1.1\n\r"
//    std::string test_random_invalid_method(mk::random_str_uppercase(4));
//    test_random_invalid_method += " / HTTP/1.1\n\r";
//    send_receive_invalid_request_line(
//        entry, *backend_url, test_random_invalid_method, handle_response,
//        options, reactor, logger);
//
//    // test_random_invalid_field_count
//    // ' '.join(randomStr(5) for x in range(4)) + '\n\r'
//    std::string test_random_invalid_field_count(mk::random_str_uppercase(5));
//    for (int i = 0; i < 3; i++) {
//        test_random_invalid_field_count += " " + mk::random_str_uppercase(5);
//    }
//    test_random_invalid_field_count += "\n\r";
//    send_receive_invalid_request_line(
//        entry, *backend_url, test_random_invalid_field_count, handle_response,
//        options, reactor, logger);
//
//    // test_random_big_request_method
//    // randomStr(1024) + ' / HTTP/1.1\n\r'
//    std::string test_random_big_request_method(mk::random_str_uppercase(1024));
//    test_random_big_request_method += " / HTTP/1.1\n\r";
//    send_receive_invalid_request_line(
//        entry, *backend_url, test_random_big_request_method, handle_response,
//        options, reactor, logger);
//
//    // test_random_invalid_version_number
//    // 'GET / HTTP/' + randomStr(3)
//    std::string test_random_invalid_version_number("GET / HTTP/");
//    test_random_invalid_version_number += mk::random_str_uppercase(3);
//    send_receive_invalid_request_line(
//        entry, *backend_url, test_random_invalid_version_number,
//        handle_response, options, reactor, logger);
}

} // namespace ooni
} // namespace mk
