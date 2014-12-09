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

  InputFileIterator(void) {}

  InputFileIterator(std::string input_filepath) {
    is = ifstream(input_filepath);
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
  InputFileIterator operator++(int)
  {
    InputFileIterator prev(*this);
    ++*this;
    return prev;
  }

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

  bool operator!=(const InputFileIterator& other) const
  {
    return is.eof != other.eof;
  }

  bool operator==(const InputFileIterator& other) const
  {
    return is.eof != other.eof;
  }
private:
  ifstream is;
  std::string value;
  bool eof;
}

class NetTest {
  
  std::string input_filepath;
  FileReporter file_report;

public:
  ight::common::Settings options;
  std::string input;

  NetTest(void);

  NetTest(std::string input_filepath);

  NetTest(std::string input_filepath, ight::common::Settings options);

  InputFileIterator input_file();

  void geoip_lookup();

  void begin();

  void write_header();

  void end();

  // XXX leave both or only one?
  virtual void main(ight::common::Settings options,
                    std::function<void(ReportEntry&&)>&& func);


  virtual void main(std::string input, ight::common::Settings options,
                    std::function<void(ReportEntry&&)>&& func);


}

}}}

#endif  // LIBIGHT_OONI_NETTEST_HPP
