// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/fcompose.hpp"
#include <measurement_kit/common.hpp>
#include <iostream>

static inline void fcompose_sync_example() {
    auto f = mk::fcompose(mk::fcompose_policy_sync(),
                          [](int x, int y, int z) { return x + y + z; },
                          [](int sum) {
                              return std::make_tuple(std::string{"result"},
                                                     std::to_string(sum));
                          },
                          [](std::string &&prefix, std::string &&result) {
                              std::cout << prefix << ": " << result << "\n";
                          });
    f(0, 4, 7);
}

static inline void fcompose_async_example() {
    // Simulate deferred callbacks using r->call_soon().
    auto r = mk::Reactor::global();
    auto f = mk::fcompose(
          mk::fcompose_policy_async(),
          [r](int x, int y, int z, mk::Callback<int> &&cb) {
              r->call_soon([
                  x = std::move(x), y = std::move(y), z = std::move(z),
                  cb = std::move(cb)
              ]() { cb(x + y + z); });
          },
          [r](int s, mk::Callback<std::string &&, std::string &&> &&cb) {
              r->call_soon([ s = std::move(s), cb = std::move(cb) ]() {
                  cb("result", std::to_string(s));
              });
          },
          [r](std::string &&p, std::string &&re, mk::Callback<> &&cb) {
              r->call_soon([
                  p = std::move(p), re = std::move(re), cb = std::move(cb)
              ]() {
                  std::cout << p << ": " << re << "\n";
                  cb();
              });
          });
    r->run_with_initial_event(
          [r, f]() mutable { f(0, 4, 7, [r]() { r->stop(); }); });
}

int main() {
    fcompose_sync_example();
    fcompose_async_example();
}
