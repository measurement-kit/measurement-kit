#include <ight/ooni/http_invalid_request_line.hpp>
#include <sys/stat.h>

using namespace ight::common::settings;
using namespace ight::ooni::http_invalid_request_line;

void
HTTPInvalidRequestLine::main(Settings options,
                             std::function<void(ReportEntry)>&& cb)
{
    
    callback = cb;
    auto handle_response = [this](Error,
                              protocols::http::Response&&) ->  void
    {
        tests_run += 1;
        if (tests_run == 3) {
            callback(entry); 
        }
        // XXX we currently don't set the tampering key, because this test
        // speaks to a TCP Echo helper, hence the response will not be valid
        // HTTP.
    };
    
    protocols::http::Headers headers;
    // test_random_invalid_method
    // randomSTR(4) + " / HTTP/1.1\n\r"
    request({
        {"url", options["backend"]},
        {"method", ight_random_str_uppercase(4)},
        {"http_version", "HTTP/1.1"},
    }, headers, "", handle_response);

    // test_random_invalid_field_count
    // ' '.join(randomStr(5) for x in range(4)) + '\n\r'
    // XXX currently cannot be implemented using HTTP client lib.


    // test_random_big_request_method
    // randomStr(1024) + ' / HTTP/1.1\n\r'
    request({
        {"url", options["backend"]},
        {"method", ight_random_str_uppercase(1024)},
        {"http_version", "HTTP/1.1"},
    }, headers, "", handle_response);

    // test_random_invalid_version_number
    // 'GET / HTTP/' + randomStr(3)
    request({
        {"url", options["backend"]},
        {"method", "GET"},
        {"http_version", "HTTP/" + ight_random_str(3)},
    }, headers, "", handle_response);
}
