// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_NETTESTS_HPP
#define MEASUREMENT_KIT_NETTESTS_NETTESTS_HPP

#include <cstdint>
#include <functional>
#include <measurement_kit/common/lexical_cast.hpp>
#include <measurement_kit/common/shared_ptr.hpp>
#include <string>

namespace mk {
namespace nettests {
class Runnable;

class BaseTest {
  public:
    BaseTest();

    virtual ~BaseTest();

    BaseTest &add_input(std::string);

    BaseTest &add_input_filepath(std::string);

    [[deprecated]] BaseTest &set_input_filepath(std::string);

    BaseTest &set_output_filepath(std::string);

    BaseTest &set_error_filepath(std::string);

    BaseTest &on_logger_eof(std::function<void()> &&);

    BaseTest &on_log(std::function<void(uint32_t, const char *)> &&);

    BaseTest &on_event(std::function<void(const char *)> &&);

    BaseTest &on_progress(std::function<void(double, const char *)> &&);

    BaseTest &set_verbosity(uint32_t);

    BaseTest &increase_verbosity();

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

    BaseTest &set_option(std::string key, std::string value);

    BaseTest &on_entry(std::function<void(std::string)> &&);

    BaseTest &on_begin(std::function<void()> &&);

    BaseTest &on_end(std::function<void()> &&);

    BaseTest &on_destroy(std::function<void()> &&);

    void run();

    void start(std::function<void()> &&);

    SharedPtr<Runnable> runnable;
};

#define MK_DECLARE_TEST(_name_)                                                \
    class _name_ : public BaseTest {                                           \
      public:                                                                  \
        _name_();                                                              \
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

} // namespace nettests
} // namespace mk
#endif
