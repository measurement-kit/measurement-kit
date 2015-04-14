/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_OONI_NET_TEST_HPP
# define IGHT_OONI_NET_TEST_HPP

#include <iterator>
#include <iostream>
#include <fstream>

#include <ight/report/file.hpp>

#include <ight/common/pointer.hpp>
#include <ight/common/poller.hpp>
#include <ight/common/settings.hpp>
#include <ight/common/log.hpp>
#include <ight/common/net_test.hpp>

namespace ight {
namespace ooni {
namespace net_test {

using namespace ight::common::libevent;
using namespace ight::common::log;
using namespace ight::common::pointer;
using namespace ight::common::poller;
using namespace ight::common::settings;

using namespace ight::report::file;

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
      SharedPointer<Logger> lp = DefaultLogger::get()) : logger(lp) {
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
  SharedPointer<Logger> logger = DefaultLogger::get();
};

class NetTest : public ight::common::net_test::NetTest {
  std::string input_filepath;
  FileReporter file_report;

  SharedPointer<DelayedCall> delayed_call;

  void run_next_measurement(const std::function<void()>&& cb);

  void geoip_lookup();

  void write_header();

  std::string get_report_filename();

protected:
  Libevent *libevent = GlobalLibevent::get();

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

}}}
#endif
