// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_NETTESTS_HPP
#define MEASUREMENT_KIT_NETTESTS_NETTESTS_HPP

#include <stdint.h>

#include <functional>
#include <string>

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/lexical_cast.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/shared_ptr.hpp>

namespace mk {

namespace nettests {
class Runnable;

class BaseTest {
  public:
    BaseTest &on_logger_eof(Callback<>);
    BaseTest &on_log(Callback<uint32_t, const char *>);
    BaseTest &on_event(Callback<const char *>);
    BaseTest &on_progress(Callback<double, const char *>);
    BaseTest &set_verbosity(uint32_t);
    BaseTest &increase_verbosity();

    BaseTest();
    virtual ~BaseTest();

    BaseTest &add_input(std::string);
    BaseTest &add_input_filepath(std::string);
    BaseTest &set_input_filepath(std::string);
    BaseTest &set_output_filepath(std::string);
    BaseTest &set_error_filepath(std::string);

    template <typename T,
              typename = typename std::enable_if<
                    !std::is_same<std::string, T>::value>::type>
    BaseTest &set_options(std::string key, T value) {
        return set_options(key, lexical_cast<std::string>(value));
    }

    BaseTest &set_options(std::string key, std::string value);

    BaseTest &on_entry(Callback<std::string>);
    BaseTest &on_begin(Callback<>);
    BaseTest &on_end(Callback<> cb);
    BaseTest &on_destroy(Callback<> cb);

    void run();
    void start(Callback<> func);

    SharedPtr<Runnable> runnable;
    Settings settings;
};

#define MK_DECLARE_TEST(_name_)                                                \
    class _name_ : public BaseTest {                                           \
      public:                                                                  \
        _name_();                                                              \
    }

MK_DECLARE_TEST(DashTest);
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
