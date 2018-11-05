// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include <measurement_kit/ooni.hpp>

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/nettests/utils_impl.hpp"

#include <sstream>
#include <unordered_set>

static mk::SharedPtr<std::istream> check_variable_expanded(const std::string &path) {
    REQUIRE(path == "it");
    return mk::SharedPtr<std::istream>{new std::stringstream{}};
}

static mk::SharedPtr<std::istream> ensure_file_openned(const std::string &path) {
    mk::SharedPtr<std::istream> result = mk::nettests::open_file_(path);
    REQUIRE(result->good());
    return result;
}

static bool cannot_read_line(std::istream &, std::string &) { return false; }

static bool simulate_io_error(std::istream &s, std::string &) {
    s.setstate(std::ios_base::failbit);
    return static_cast<bool>(s);
}

static void intercept_randomize(std::deque<std::string> &) {
    throw mk::MockedError();
}

TEST_CASE("process_input_filepaths() works as expected") {

    SECTION("When needs_input and no input filepaths are available "
            "and no manual input is available") {
        std::deque<std::string> inputs;
        mk::Error error =
            mk::nettests::process_input_filepaths(inputs,
                true, {}, "IT", {}, mk::Logger::global(), nullptr, nullptr);
        REQUIRE(error);
        REQUIRE(error ==
                mk::ooni::MissingRequiredInputFileError());
    }

    SECTION("When needs_input and no input filepaths are available "
            "but manual input is available") {
        /* The opposite case, where there is no manual input but we have
           files containing input is tested down below. */
        std::deque<std::string> inputs{"antani"};
        mk::Error error =
            mk::nettests::process_input_filepaths(inputs,
                true, {}, "IT", {}, mk::Logger::global(), nullptr, nullptr);
        REQUIRE(!error);
        REQUIRE(error == mk::NoError());
    }

    SECTION("The ${probe_cc} variable is correctly expanded") {
        std::deque<std::string> inputs;
        mk::Error error =
            mk::nettests::process_input_filepaths_impl<check_variable_expanded>(
                inputs,
                true, {"${probe_cc}"}, "IT", {}, mk::Logger::global(), nullptr,
                nullptr);
        /*
         * We are going to fail because the mocked function returns a
         * stream that is already at EOF.
         */
        REQUIRE(error);
        REQUIRE(error ==
                mk::ooni::CannotReadAnyInputFileError());
    }

    SECTION("When no line could be read, an error is returned") {
        std::deque<std::string> inputs;
        mk::Error error =
            mk::nettests::process_input_filepaths_impl<ensure_file_openned,
                                                       cannot_read_line>(
                inputs,
                true, {"./test/fixtures/urls.txt"}, "IT", {},
                mk::Logger::global(), nullptr, nullptr);
        REQUIRE(error);
        REQUIRE(error ==
                mk::ooni::CannotReadAnyInputFileError());
    }

    SECTION("If we can't open a file, the proper function is notified") {
        std::deque<std::string> inputs;
        std::string cannot_open;
        mk::Error error =
            mk::nettests::process_input_filepaths(inputs,
                true, {"./nonexistent"}, "IT", {}, mk::Logger::global(),
                [&](const std::string &p) { cannot_open = p; }, nullptr);
        REQUIRE(error);
        REQUIRE(error ==
                mk::ooni::CannotReadAnyInputFileError());
        REQUIRE(cannot_open == "./nonexistent");
    }

    SECTION("We are notified if there is I/O error") {
        std::deque<std::string> inputs;
        std::string cannot_read;
        mk::Error error =
            mk::nettests::process_input_filepaths_impl<mk::nettests::open_file_,
                                                       simulate_io_error>(
                inputs, true, {"./test/fixtures/urls.txt"}, "IT", {},
                mk::Logger::global(), nullptr,
                [&](const std::string &p) { cannot_read = p; });
        REQUIRE(error);
        REQUIRE(error ==
                mk::ooni::CannotReadAnyInputFileError());
        REQUIRE(cannot_read == "./test/fixtures/urls.txt");
    }

    SECTION("When the randomize_input option is invalid") {
        std::deque<std::string> inputs;
        mk::Error error =
            mk::nettests::process_input_filepaths(
                inputs, true, {"./test/fixtures/urls.txt"}, "IT",
                {{"randomize_input", "antani"}}, mk::Logger::global(), nullptr,
                nullptr);
        REQUIRE(error);
        REQUIRE(error == mk::ValueError());
    }

    SECTION("The randomize functionality is invoked when requested") {
        std::deque<std::string> inputs;
        REQUIRE_THROWS_AS(
            (mk::nettests::process_input_filepaths_impl<
                mk::nettests::open_file_, mk::nettests::readline_,
                intercept_randomize>(inputs, true, {"./test/fixtures/urls.txt"}, "IT",
                                     {{"randomize_input", "1"}},
                                     mk::Logger::global(), nullptr, nullptr)),
            mk::MockedError);
    }

    SECTION("The randomize functionality is invoked by default") {
        std::deque<std::string> inputs;
        REQUIRE_THROWS_AS((mk::nettests::process_input_filepaths_impl<
                              mk::nettests::open_file_, mk::nettests::readline_,
                              intercept_randomize>(inputs,
                              true, {"./test/fixtures/urls.txt"}, "IT", {},
                              mk::Logger::global(), nullptr, nullptr)),
                          mk::MockedError);
    }

    SECTION("The randomize functionality is not invoked when not requested") {
        std::deque<std::string> inputs;
        /* No randomize, hence here we should not see exceptions: */
        mk::nettests::process_input_filepaths_impl<mk::nettests::open_file_,
                                                   mk::nettests::readline_,
                                                   intercept_randomize>(
            inputs, true, {"./test/fixtures/urls.txt"}, "IT",
            {{"randomize_input", "0"}}, mk::Logger::global(), nullptr, nullptr);
    }

    SECTION("The randomize primitive works as expected") {

        /*
         * Same approach used in test/nettests/runnable.cpp: it is random
         * if within a reasonable number of permutations we get back a
         * vector different from the original one.
         */

        std::deque<std::string> expect{"torproject.org",
                                       "ooni.nu",
                                       "neubot.org",
                                       "archive.org",
                                       "creativecommons.org",
                                       "cyber.law.harvard.edu",
                                       "duckduckgo.com",
                                       "netflix.com",
                                       "nmap.org",
                                       "www.emule.com"};

        std::deque<std::string> copy = expect;
        auto count = 0;
        for (; count < 8; ++count) {
            mk::nettests::randomize_input_(copy);
            if (expect != copy) {
                return;
            }
        }
        REQUIRE(false);
    }

    SECTION("When input is not required, just a single entry is returned") {
        std::deque<std::string> inputs;
        mk::Error error =
            mk::nettests::process_input_filepaths(
                inputs, false, {"./test/fixtures/urls.txt"}, "IT",
                {{"randomize_input", "antani"}}, mk::Logger::global(), nullptr,
                nullptr);
        REQUIRE(!error);
        REQUIRE(inputs.size() == 1);
        REQUIRE(inputs == std::deque<std::string>{{""}});
    }

    SECTION("We deal correctly with manual input when no input is expected") {
        std::deque<std::string> inputs{"antani", "mascetti"};
        mk::Error error =
            mk::nettests::process_input_filepaths(
                inputs, false, {"./test/fixtures/urls.txt"}, "IT",
                {{"randomize_input", "antani"}}, mk::Logger::global(), nullptr,
                nullptr);
        REQUIRE(!error);
        REQUIRE(inputs.size() == 1);
        REQUIRE(inputs == std::deque<std::string>{{""}});
    }

    SECTION("It is able to load and shuffle input") {
        std::deque<std::string> inputs;
        std::unordered_set<std::string> expect{"http://torproject.org",
                                               "http://ooni.nu",
                                               "http://neubot.org",
                                               "http://archive.org",
                                               "http://creativecommons.org",
                                               "http://cyber.law.harvard.edu",
                                               "http://duckduckgo.com",
                                               "http://netflix.com",
                                               "http://nmap.org",
                                               "http://whatismyipaddress.com",
                                               "http://www.emule.com"};
        mk::Error error =
            mk::nettests::process_input_filepaths(inputs,
                true, {"./test/fixtures/urls.txt"}, "IT", {},
                mk::Logger::global(), nullptr, nullptr);
        REQUIRE(!error);
        std::unordered_set<std::string> result(inputs.begin(),
                                               inputs.end());
        REQUIRE(expect == result);
    }
}
