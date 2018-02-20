// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_NETTESTS_HPP
#define MEASUREMENT_KIT_NETTESTS_NETTESTS_HPP

#include <stdint.h>

#include <functional>
#include <string>
#include <type_traits>

#include <measurement_kit/common/data_usage.hpp>
#include <measurement_kit/common/shared_ptr.hpp>

namespace mk {
namespace nettests {
class Runnable;

class BaseTest {
  public:
    BaseTest();

    virtual ~BaseTest();

    BaseTest &add_input(std::string);

    BaseTest &add_input_filepath(std::string);

    BaseTest &set_output_filepath(std::string);

    BaseTest &set_error_filepath(std::string);

    BaseTest &on_logger_eof(std::function<void()> &&);

    BaseTest &on_log(std::function<void(uint32_t, const char *)> &&);

    BaseTest &on_event(std::function<void(const char *)> &&);

    BaseTest &on_progress(std::function<void(double, const char *)> &&);

    BaseTest &set_verbosity(uint32_t);

    BaseTest &increase_verbosity();

    template <typename T, typename = typename std::enable_if<
                    std::is_arithmetic<T>::value>::type>
    BaseTest &set_option(std::string key, T value) {
        return set_option(key, std::to_string(value));
    }

    BaseTest &set_option(std::string key, std::string value);

    BaseTest &add_annotation(std::string key, std::string value);

    BaseTest &on_entry(std::function<void(std::string)> &&);

    BaseTest &on_begin(std::function<void()> &&);

    BaseTest &on_end(std::function<void()> &&);

    BaseTest &on_destroy(std::function<void()> &&);

    // `on_overall_data_usage` allows you to register a callback to be called
    // only once, at the end of the test with information on the overall amount
    // of data (upload and download) used during the this test.
    BaseTest &on_overall_data_usage(std::function<void(DataUsage)> &&);

    void run();

    void start(std::function<void()> &&);

    SharedPtr<Runnable> runnable;
};

#define MK_ENUM_TEST(XX)                                                       \
    XX(DashTest)                                                               \
    XX(CaptivePortalTest)                                                      \
    XX(DnsInjectionTest)                                                       \
    XX(FacebookMessengerTest)                                                  \
    XX(HttpHeaderFieldManipulationTest)                                        \
    XX(HttpInvalidRequestLineTest)                                             \
    XX(MeekFrontedRequestsTest)                                                \
    XX(MultiNdtTest)                                                           \
    XX(NdtTest)                                                                \
    XX(TcpConnectTest)                                                         \
    XX(TelegramTest)                                                           \
    XX(WebConnectivityTest)                                                    \
    XX(WhatsappTest)

#define MK_DECLARE_TEST(_name_)                                                \
    class _name_ : public BaseTest {                                           \
      public:                                                                  \
        _name_();                                                              \
    };
MK_ENUM_TEST(MK_DECLARE_TEST)
#undef MK_DECLARE_TEST

} // namespace nettests
} // namespace mk
#endif
