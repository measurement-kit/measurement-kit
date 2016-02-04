// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_INPUT_GENERATOR_HPP
#define SRC_OONI_INPUT_GENERATOR_HPP

#include <functional>
#include <string>

namespace mk {
namespace ooni {

class InputGenerator {

  public:
    virtual void next(std::function<void(std::string)> &&new_line,
                      std::function<void()> &&done) = 0;

    virtual ~InputGenerator() {}
};

} // namespace ooni
} // namespace mk
#endif
