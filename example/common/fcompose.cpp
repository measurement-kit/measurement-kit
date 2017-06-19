// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>

#include <iostream>

static inline void fcompose_sync_example() {
    auto func = mk::fcompose(mk::fcompose_policy_async_and_route_exceptions(
                                   [](const std::exception &exc) {
                                       std::clog
                                             << "Error occurred: " << exc.what()
                                             << "\n";
                                   }),
                             sum3, tostr, print_fail);
    func(0, -4, -7, []() { /* NOTHING */ });
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
    r->run_with_initial_event([r, f]() { f(0, 4, 7, [r]() { r->stop(); }); });
}

int main() {
    fcompose_sync_example();
    fcompose_async_example();
}
