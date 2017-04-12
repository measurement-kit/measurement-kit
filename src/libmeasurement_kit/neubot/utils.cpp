// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../neubot/utils.hpp"

namespace mk {
namespace neubot {

std::vector<int> dash_rates() {
    static const std::vector<int> rates{
        {100,  150,  200,  250,  300,  400,  500,  700,  900,   1200,
         1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 10000, 20000}};
    return rates;
}

} // namespace neubot
} // namespace mk
