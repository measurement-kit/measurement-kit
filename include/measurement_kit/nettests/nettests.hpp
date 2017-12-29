// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_NETTESTS_HPP
#define MEASUREMENT_KIT_NETTESTS_NETTESTS_HPP

#include <cstdint>
#include <functional>
#include <measurement_kit/common/api.h>
#include <measurement_kit/common/data_usage.hpp>
#include <measurement_kit/common/lexical_cast.hpp>
#include <measurement_kit/common/shared_ptr.hpp>
#include <string>

namespace mk {
namespace nettests {
class Runnable;

class BaseTest {
  public:
    MK_API BaseTest();

    MK_API virtual ~BaseTest();

    MK_API BaseTest &add_input(std::string);

    MK_API BaseTest &add_input_filepath(std::string);

    [[deprecated]] BaseTest &set_input_filepath(std::string);

    MK_API BaseTest &set_output_filepath(std::string);

    MK_API BaseTest &set_error_filepath(std::string);

    MK_API BaseTest &on_logger_eof(std::function<void()> &&);

    MK_API BaseTest &on_log(std::function<void(uint32_t, const char *)> &&);

    MK_API BaseTest &on_event(std::function<void(const char *)> &&);

    MK_API BaseTest &on_progress(std::function<void(double, const char *)> &&);

    MK_API BaseTest &set_verbosity(uint32_t);

    MK_API BaseTest &increase_verbosity();

    template <typename T,
              typename = typename std::enable_if<
                    !std::is_same<std::string, T>::value>::type>
    [[deprecated]] BaseTest &set_options(std::string key, T value) {
        return set_option(key, mk::lexical_cast<std::string>(value));
    }

    [[deprecated]] BaseTest &set_options(std::string key, std::string value);

    template <typename T,
              typename = typename std::enable_if<
                    !std::is_same<std::string, T>::value>::type>
    BaseTest &set_option(std::string key, T value) {
        return set_option(key, mk::lexical_cast<std::string>(value));
    }

    MK_API BaseTest &add_annotation(std::string key, std::string value);

    MK_API BaseTest &set_option(std::string key, std::string value);

    MK_API BaseTest &on_entry(std::function<void(std::string)> &&);

    MK_API BaseTest &on_begin(std::function<void()> &&);

    MK_API BaseTest &on_end(std::function<void()> &&);

    MK_API BaseTest &on_destroy(std::function<void()> &&);

    // `on_overall_data_usage` allows you to register a callback to be called
    // only once, at the end of the test with information on the overall amount
    // of data (upload and download) used during the this test.
    MK_API BaseTest &on_overall_data_usage(std::function<void(DataUsage)> &&);

    MK_API void run();

    MK_API void start(std::function<void()> &&);

    SharedPtr<Runnable> runnable;
};

// XXX We should probably also define the virtual destructor?
#define MK_DECLARE_TEST(_name_)                                                \
    class _name_ : public BaseTest {                                           \
      public:                                                                  \
        MK_API _name_();                                                       \
    }

MK_DECLARE_TEST(DashTest);
MK_DECLARE_TEST(CaptivePortalTest);
MK_DECLARE_TEST(DnsInjectionTest);
MK_DECLARE_TEST(FacebookMessengerTest);
MK_DECLARE_TEST(HttpHeaderFieldManipulationTest);
MK_DECLARE_TEST(HttpInvalidRequestLineTest);
MK_DECLARE_TEST(MeekFrontedRequestsTest);
MK_DECLARE_TEST(MultiNdtTest);
MK_DECLARE_TEST(NdtTest);
MK_DECLARE_TEST(TcpConnectTest);
MK_DECLARE_TEST(TelegramTest);
MK_DECLARE_TEST(WebConnectivityTest);
MK_DECLARE_TEST(WhatsappTest);

} // namespace nettests
} // namespace mk
#endif
