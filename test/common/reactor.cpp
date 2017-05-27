// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

class Task {
  public:
    Task(Var<Reactor> r) : reactor_(r) {}
    void operator()(Callback<> &&cb) const {
        reactor_->call_later(1.0, std::move(cb));
    }

  private:
    Var<Reactor> reactor_;
};

TEST_CASE("Reactor::run_task() works") {
    auto reactor = Reactor::make();
    auto logger = Logger::make();
    logger->set_verbosity(MK_LOG_DEBUG);

    SECTION("With lambda") {
        reactor->run_with_initial_event([reactor, logger]() {
            reactor->run_task_deferred(
                "lambda", logger,
                [reactor](Callback<> &&cb) {
                    reactor->call_later(1.0, std::move(cb));
                },
                [reactor](auto && /*task*/) { reactor->break_loop(); });
        });
    }

    SECTION("With object") {
        reactor->run_with_initial_event([reactor, logger]() {
            reactor->run_task_deferred(
                "object", logger, Task{reactor},
                [reactor](const Task && /*task*/) { reactor->break_loop(); });
        });
    }
}
