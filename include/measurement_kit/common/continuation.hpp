// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_CONTINUATION_HPP
#define MEASUREMENT_KIT_COMMON_CONTINUATION_HPP

#include <measurement_kit/common/callback.hpp>

namespace mk {

/// \brief `Continuation` is a function that will call another function
/// when complete. In several places we need to return code to be executed
/// later, perhaps in parallel with other code. In such cases, we usually
/// return a function that captures relevant parameters and receives in
/// input a callback to be called later when done. The `Continuation` alias
/// allows to express the returned function more compactly.
///
/// For example:
///
/// ```C++
///
/// static Continuation<Error> measure_foo(std::string address) {
///     return [address = std::move(address)](Callback<Error> &&cb) {
///         foo_start(std::move(address), std::move(cb));
///     };
/// }
///
/// static Continuation<Error> measure_foobar(std::string address) {
///     return [address = std::move(address)](Callback<Error> &&cb) {
///         foobar_start(std::move(address), std::move(cb));
///     };
/// }
///
/// ParallelExecutor{}
///     .add(measure_foo(address))
///     .add(measure_foobar(address))
///     .start([](Error err) {
///         // This will be called when both are done
///     });
///
/// ```
///
/// \since v0.2.0.
template <typename... T>
using Continuation = std::function<void(Callback<T...>)>;

} // namespace mk
#endif
