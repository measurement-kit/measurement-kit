#ifndef LIBIGHT_OONI_NETTEST_HPP
# define LIBIGHT_OONI_NETTEST_HPP

#include <iterator>
#include <iostream>
#include <fstream>

#include "report/file.hpp"
#include "common/settings.hpp"

namespace ight {
namespace ooni {
namespace nettest {

class InputFileIterator: public std::iterator<std::input_iterator_tag, std::string,
  std::ptrdiff_t, const std::string*, const std::string&>
{
public:
  bool eof;

  InputFileIterator(void) {}

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

  InputFileIterator begin()
  {
    InputFileIterator o;
    o.eof = false;
    return o;
  }

  InputFileIterator end()
  {
    InputFileIterator o;
    o.eof = true;
    return o;
  }

  const std::string& operator*() const { return value; }
  const std::string* operator->() const { return &value; }

  InputFileIterator& operator++()
  {
    if (is && !getline(*is, value)) {
      eof = true;
    }
    return *this;
  }

  bool operator!=(const InputFileIterator& other) const
  {
    return eof != other.eof;
  }

  bool operator==(const InputFileIterator& other) const
  {
    return eof != other.eof;
  }

private:
  std::ifstream *is = nullptr;
  std::string value;
};

class NetTest {
  
  std::string input_filepath;
  FileReporter file_report;

public:
  ight::common::Settings options;
  InputFileIterator input;

  NetTest(void);

  NetTest(std::string input_filepath);

  NetTest(std::string input_filepath, ight::common::Settings options);

  InputFileIterator input_file();

  void run_next_measurement(std::function<void()>&& cb);

  void geoip_lookup();

  void begin(std::function<void()>&& cb);

  void write_header();

  void end(std::function<void()>&& cb);

  // XXX leave both or only one?
  virtual void main(ight::common::Settings options,
                    std::function<void(ReportEntry&&)>&& func);


  virtual void main(std::string input, ight::common::Settings options,
                    std::function<void(ReportEntry&&)>&& func);


};

}}}

#endif  // LIBIGHT_OONI_NETTEST_HPP
