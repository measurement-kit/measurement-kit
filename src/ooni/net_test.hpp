#ifndef LIBIGHT_OONI_NETTEST_HPP
# define LIBIGHT_OONI_NETTEST_HPP

#include <iterator>
#include <iostream>
#include <fstream>

#include "report/file.hpp"
#include "common/poller.h"
#include "common/settings.hpp"

namespace ight {
namespace ooni {
namespace net_test {

class InputFileIterator: public std::iterator<std::input_iterator_tag, std::string,
  std::ptrdiff_t, const std::string*, const std::string&>
{
public:

  InputFileIterator() {}

  InputFileIterator(std::string input_filepath) {
    is = new std::ifstream(input_filepath);
  }

  ~InputFileIterator() {
    delete is;  /* delete handles nullptr */
  }

  InputFileIterator(InputFileIterator&) = delete;
  InputFileIterator& operator=(InputFileIterator&) = delete;
  InputFileIterator(InputFileIterator&&) = default;
  InputFileIterator& operator=(InputFileIterator&&) = default;

  InputFileIterator begin() {
    InputFileIterator o;
    o.eof = false;
    return o;
  }

  InputFileIterator end() {
    InputFileIterator o;
    o.eof = true;
    return o;
  }

  const std::string& operator*() const {
    return value;
  }

  const std::string* operator->() const {
    return &value;
  }

  InputFileIterator& operator++() {
    if (is == nullptr) {
      throw std::runtime_error("Iterator not bound to any file");
    }
    if (!getline(*is, value)) {
      eof = true;
    }
    return *this;
  }

  bool operator!=(const InputFileIterator& other) const {
    return eof != other.eof;
  }

  bool operator==(const InputFileIterator& other) const {
    return eof == other.eof;
  }

private:
  std::ifstream *is = nullptr;
  std::string value;
  bool eof = false;
};

class NetTest {
  std::string input_filepath;
  FileReporter file_report;

  IghtDelayedCall delayed_call;

  void run_next_measurement(std::function<void()>&& cb);

  void geoip_lookup();

  void write_header();

protected:
  IghtLibevent *libevent = IghtGlobalLibevent::get();

  virtual void setup(std::string input);
  virtual void setup();

  virtual void teardown(std::string input);
  virtual void teardown();

  virtual void main(ight::common::Settings options,
                    std::function<void(ReportEntry)>&& func);


  virtual void main(std::string input, ight::common::Settings options,
                    std::function<void(ReportEntry)>&& func);

public:
  ReportEntry entry;
  ight::common::Settings options;
  InputFileIterator input;

  NetTest(void);

  NetTest(NetTest&) = delete;
  NetTest& operator=(NetTest&) = delete;
  NetTest(NetTest&&) = default;
  NetTest& operator=(NetTest&&) = default;

  NetTest(std::string input_filepath);

  NetTest(std::string input_filepath, ight::common::Settings options);

  InputFileIterator input_file();

  /*!
   * \brief Start iterating over the input.
   * \param cb Callback called when we are done.
   */
  void begin(std::function<void()>&& cb);

  /*!
   * \brief Make sure that the report is written.
   * \param cb Callback called when the report is written.
   */
  void end(std::function<void()>&& cb);
};

}}}

#endif  // LIBIGHT_OONI_NETTEST_HPP
