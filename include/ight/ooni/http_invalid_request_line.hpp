#ifndef LIBIGHT_OONI_HTTP_INVALID_REQUEST_LINE_HPP
# define LIBIGHT_OONI_HTTP_INVALID_REQUEST_LINE_HPP

#include <ight/common/utils.hpp>
#include <ight/protocols/http.hpp>
#include <ight/ooni/net_test.hpp>
#include <ight/ooni/http_test.hpp>
#include <sys/stat.h>

using namespace ight::ooni::http_test;

namespace ight {
namespace ooni {
namespace http_invalid_request_line {

class InputFileDoesNotExist : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class HTTPInvalidRequestLine: public HTTPTest {
    using HTTPTest::HTTPTest;
    
    int tests_run = 0;

    std::function<void(ReportEntry)> callback;

public:
    HTTPInvalidRequestLine(ight::common::Settings options_) : 
      HTTPTest(options_) {
        test_name = "http_invalid_request_line";
        test_version = "0.0.1";
    };
    
    void main(ight::common::Settings options,
              std::function<void(ReportEntry)>&& cb);
};

}}}

#endif  // LIBIGHT_OONI_HTTP_INVALID_REQUEST_LINE_HPP
