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

static inline void sum3(int x, int y, int z, mk::Callback<int> cb) {
    cb(x + y + z);
}

static inline void tostr(int sum,
                         mk::Callback<std::string &&, std::string &&> cb) {
    cb("result", std::to_string(sum));
}

static inline void print(std::string &&prefix, std::string &&result,
                         mk::Callback<> cb) {
    std::cout << prefix << ": " << result << "\n";
    cb();
}

static inline void fcompose_async_example1() {
    auto func = mk::fcompose(mk::fcompose_policy_async(), sum3, tostr, print);
    func(0, 4, 7, []() { /* NOTHING */ });
}

static inline void fcompose_async_example2() {
    auto r = mk::Reactor::global();
    auto func = mk::fcompose(
          mk::fcompose_policy_async(),
          [=](int x, int y, int z, mk::Callback<int> cb) {
              r->call_soon([=]() { sum3(x, y, z, cb); });
          },
          [=](int sum, mk::Callback<std::string &&, std::string &&> cb) {
              r->call_soon([=]() { tostr(sum, cb); });
          },
          [=](std::string &&a, std::string &&b, mk::Callback<> cb) {
              r->call_soon(
                    [=]() mutable { print(std::move(a), std::move(b), cb); });
          });
    r->run_with_initial_event([=]() { func(0, 4, 7, [=]() { r->stop(); }); });
}

static inline void print_fail(std::string &&, std::string &&, mk::Callback<>) {
    throw mk::MockedError("simulating error along the chain");
}

static inline void fcompose_async_and_route_exceptions_example1() {
    auto func = mk::fcompose(mk::fcompose_policy_async_and_route_exceptions(
                                   [](const std::exception &exc) {
                                       std::clog
                                             << "Error occurred: " << exc.what()
                                             << "\n";
                                   }),
                             sum3, tostr, print_fail);
    func(0, -4, -7, []() { /* NOTHING */ });
}

static inline void fcompose_async_and_route_exceptions_example2() {
    auto r = mk::Reactor::global();
    auto func = mk::fcompose(
          mk::fcompose_policy_async_and_route_exceptions(
                [](const std::exception &exc) {
                    std::clog << "Error occurred: " << exc.what() << "\n";
                }),
          [=](int x, int y, int z, mk::Callback<int> cb) {
              r->call_soon([=]() { sum3(x, y, z, cb); });
          },
          [=](int sum, mk::Callback<std::string &&, std::string &&> cb) {
              r->call_soon([=]() { tostr(sum, cb); });
          },
          [=](std::string &&a, std::string &&b, mk::Callback<> cb) {
              r->call_soon([=]() mutable {
                  print_fail(std::move(a), std::move(b), cb);
              });
          });
    r->run_with_initial_event([=]() { func(0, 4, 7, [=]() { r->stop(); }); });
}

int main() {
    fcompose_sync_example();
    fcompose_async_example1();
    fcompose_async_example2();
    fcompose_async_and_route_exceptions_example1();
    fcompose_async_and_route_exceptions_example2();
}
