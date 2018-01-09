// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_SCALAR_HPP
#define MEASUREMENT_KIT_COMMON_SCALAR_HPP

#include <measurement_kit/common/error_or.hpp>
#include <sstream>

namespace mk {

/// \brief `Scalar` wraps a scalar value. Internally we use std::stringstream to
/// convert the scalar value to a string. This allows us to access the original
/// scalar value later. \see lexical_cast() for a discussion of converting
/// between scalar values using std::stringstream.
///
/// \since v0.2.0.
///
/// Before MK v0.8.0 Scalar was named SettingsEntry.
class Scalar : public std::string {
  public:

    /// \brief The default constructor constructs an empty scalar. This
    /// basicall means that internall we will store an empty string.
    Scalar() {}

    /// \brief The constructor with type constructs a scalar of the
    /// specified type with the specified \p value.
    /// \param value the value with which to initialize the scalar.
    template <typename Type> Scalar(Type value) {
        std::stringstream ss;
        ss << value;
        assign(ss.str());
    }

    /// \brief `as()` converts the scalar into the specified type.
    /// \throw ValueError if the conversion is not possible.
    /// \return the converted value otherwise.
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

    /// \brief `as_noexcept()` is like except but, rather than throwing
    /// on error, returns the error that occurred.
    template <typename Type> ErrorOr<Type> as_noexcept() const noexcept {
        try {
            return {NoError(), as<Type>()};
        } catch (const Error &e) {
            return {e, {}};
        }
    }

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace mk
#endif
