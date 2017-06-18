// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>

#include <iostream>

static inline void fcompose_sync_example() {
    auto func = mk::fcompose(mk::fcompose_policy_sync(),
                             [](int x, int y, int z) { return x + y + z; },
                             [](int sum) {
                                 return std::make_tuple(std::string{"result"},
                                                        std::to_string(sum));
                             },
                             [](std::string &&prefix, std::string &&result) {
                                 std::cout << prefix << ": " << result << "\n";
                             });
    func(0, 4, 7);
}

static inline void fcompose_async_example() {
    auto func = mk::fcompose(
          mk::fcompose_policy_async(),
          [](int x, int y, int z, mk::Callback<int> &&cb) { cb(x + y + z); },
          [](int sum, mk::Callback<std::string &&, std::string &&> cb) {
              cb(std::string{"result"}, std::to_string(sum));
          },
          [](std::string &&prefix, std::string &&result, mk::Callback<> &&cb) {
              std::cout << prefix << ": " << result << "\n";
              cb();
          });
    func(0, 4, 7, []() { /* NOTHING */ });
}

static inline void fcompose_async_robust_example() {
    auto func = mk::fcompose(
          mk::fcompose_policy_async_robust([](const std::exception &exc) {
              std::clog << "Error occurred: " << exc.what() << "\n";
          }),
          [](int x, int y, int z, mk::Callback<int> &&cb) {
              cb(x + y + z);
          },
          [](int sum, mk::Callback<std::string &&, std::string &&> cb) {
              if (sum < 0) {
                  throw mk::ValueError("sum is negative");
              }
              cb(std::string{"result"}, std::to_string(sum));
          },
          [](std::string &&prefix, std::string &&result, mk::Callback<> &&cb) {
              std::cout << prefix << ": " << result;
              cb();
          });
    func(0, -4, -7, []() { /* NOTHING */ });
}

int main() {
    fcompose_sync_example();
    fcompose_async_example();
    fcompose_async_robust_example();
}
