// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_NET_TEST_HPP
#define MEASUREMENT_KIT_OONI_NET_TEST_HPP

#include <iterator>
#include <iostream>
#include <fstream>

#include <measurement_kit/report/file_reporter.hpp>

#include <measurement_kit/common/delayed_call.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/net_test.hpp>

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::common;
using namespace measurement_kit::report;

class InputGenerator {

public:
    virtual void
    next(std::function<void(std::string)>&& new_line,
         std::function<void()>&& done) = 0;

    virtual ~InputGenerator() {
    }
};

class InputFileGenerator : public InputGenerator
{
public:
  InputFileGenerator() {}

  InputFileGenerator(std::string input_filepath,
      Logger *lp = Logger::global()) : logger(lp) {
    is = new std::ifstream(input_filepath);
  }

  virtual ~InputFileGenerator() {
    delete is;  /* delete handles nullptr */
  }

  InputFileGenerator(InputFileGenerator&) = delete;
  InputFileGenerator& operator=(InputFileGenerator&) = delete;
  InputFileGenerator(InputFileGenerator&&) = default;
  InputFileGenerator& operator=(InputFileGenerator&&) = default;

  void
  next(std::function<void(std::string)>&& new_line,
       std::function<void()>&& done) override {
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

class NetTest : public measurement_kit::common::NetTest {
  std::string input_filepath;
  FileReporter file_report;

  DelayedCall delayed_call;

  void run_next_measurement(const std::function<void()>&& cb);

  void geoip_lookup();

  void write_header();

  std::string get_report_filename();

protected:
  Libs *libs = Libs::global();

  virtual void setup(std::string input);
  virtual void setup();

  virtual void teardown(std::string input);
  virtual void teardown();

  virtual void main(Settings options,
                    std::function<void(ReportEntry)>&& func);


  virtual void main(std::string input, Settings options,
                    std::function<void(ReportEntry)>&& func);

public:
  ReportEntry entry;
  Settings options;
  InputGenerator* input = nullptr;

  std::string test_name;
  std::string test_version;

  std::string probe_ip = "127.0.0.1";
  std::string probe_asn = "AS0";
  std::string probe_cc = "ZZ";

  NetTest(void);

  virtual ~NetTest(void);

  NetTest(NetTest&) = delete;
  NetTest& operator=(NetTest&) = delete;
  NetTest(NetTest&&) = default;
  NetTest& operator=(NetTest&&) = default;

  NetTest(std::string input_filepath);

  NetTest(std::string input_filepath, Settings options);

  InputGenerator* input_generator();

  /*!
   * \brief Start iterating over the input.
   * \param cb Callback called when we are done.
   */
  virtual void begin(std::function<void()> cb) override;

  /*!
   * \brief Make sure that the report is written.
   * \param cb Callback called when the report is written.
   */
  virtual void end(std::function<void()> cb) override;
};

}}
#endif
