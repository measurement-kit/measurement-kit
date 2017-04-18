// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_FACEBOOK_MESSENGER_HPP
#define MEASUREMENT_KIT_NETTESTS_FACEBOOK_MESSENGER_HPP

#include <measurement_kit/nettests/base_test.hpp>

namespace mk {
namespace nettests {

class FacebookMessengerTest : public BaseTest {
  public:
    FacebookMessengerTest();
};

class FacebookMessengerRunnable : public Runnable {
  public:
    void main(std::string, Settings, Callback<Var<report::Entry>>) override;
};

} // namespace nettests
} // namespace mk
#endif
