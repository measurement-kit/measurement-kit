// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_INPUT_FILE_GENERATOR_HPP
#define SRC_OONI_INPUT_FILE_GENERATOR_HPP

#include <fstream>                            // for char_traits, ifstream
#include <functional>                         // for function
#include <measurement_kit/common/logger.hpp>  // for Logger
#include "src/ooni/input_generator.hpp"       // for InputGenerator

namespace mk {
namespace ooni {

class InputFileGenerator : public InputGenerator {
  public:
    InputFileGenerator() {}

    InputFileGenerator(std::string input_filepath,
                       Logger *lp = Logger::global())
        : logger(lp) {
        is = new std::ifstream(input_filepath);
    }

    virtual ~InputFileGenerator() { delete is; /* delete handles nullptr */ }

    InputFileGenerator(InputFileGenerator &) = delete;
    InputFileGenerator &operator=(InputFileGenerator &) = delete;
    InputFileGenerator(InputFileGenerator &&) = default;
    InputFileGenerator &operator=(InputFileGenerator &&) = default;

    void next(std::function<void(std::string)> &&new_line,
              std::function<void()> &&done) override {
        logger->debug("InputFileGenerator: getting next line");
        std::string line;
        if (*is && !std::getline(*is, line).eof()) {
            logger->debug("InputFileGenerator: returning new line");
            new_line(line);
        } else {
            logger->debug("InputFileGenerator: EOF reached.");
            done();
        }
    }

  private:
    std::ifstream *is = nullptr;
    Logger *logger = Logger::global();
};

} // namespace ooni
} // namespace mk
#endif
