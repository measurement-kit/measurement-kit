// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_NOEXCEPTIONS_HPP
#define MEASUREMENT_KIT_COMMON_NOEXCEPTIONS_HPP

#include <cassert>
#include <measurement_kit/common/error_or.hpp>

namespace mk {

template <typename T> ErrorOr<T> json_noexcept(std::function<T()> fn) {
    try {
        return fn();
    } catch (std::invalid_argument &) {
        return JsonParseError();
    } catch (std::out_of_range &) {
        return JsonKeyError();
    } catch (std::domain_error &) {
        return JsonDomainError();
    } catch (Error &err) {
        return err;
    } catch (std::exception &ex) {
        return UncaughtExceptionError(); // define a specific one
    }
}

} // namespace mk

#endif
