// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_SCALAR_HPP
#define MEASUREMENT_KIT_COMMON_SCALAR_HPP

#include <measurement_kit/common/error_or.hpp>
#include <sstream>

namespace mk {

class Scalar : public std::string {
  public:
    Scalar() {}

    template <typename Type> Scalar(Type value) {
        std::stringstream ss;
        ss << value;
        assign(ss.str());
    }

    template <typename Type> Type as() const {
        std::stringstream ss{c_str()};
        Type value{};
        ss >> value;
        if (!ss.eof()) {
            throw ValueError(); // Not all input was converted
        }
        if (ss.fail()) {
            throw ValueError(); // Input format was wrong
        }
        return value;
    }

    template <typename Type> ErrorOr<Type> as_noexcept() const noexcept {
        try {
            return as<Type>();
        } catch (const Error &e) {
            return e;
        }
    }

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace mk
#endif
