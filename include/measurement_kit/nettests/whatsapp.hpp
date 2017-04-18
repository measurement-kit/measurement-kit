// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_WHATSAPP_HPP
#define MEASUREMENT_KIT_NETTESTS_WHATSAPP_HPP

#include <measurement_kit/nettests/base_test.hpp>

namespace mk {
namespace nettests {

class WhatsappTest : public BaseTest {
  public:
    WhatsappTest();
};

class WhatsappRunnable : public Runnable {
  public:
    void main(std::string, Settings, Callback<Var<report::Entry>>) override;
};

} // namespace nettests
} // namespace mk
#endif
