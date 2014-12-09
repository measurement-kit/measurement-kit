#ifndef LIBIGHT_OONI_NETTEST_HPP
# define LIBIGHT_OONI_NETTEST_HPP

#include <iterator>
#include <iostream>
#include <fstream>
#include "report/file.hpp"

namespace ight {
namespace ooni {
namespace nettest {

// FIXME to use pointer instead of functions
class InputFileIterator: public std::iterator<std::output_iterator_tag, std::string>
{
  std::string input_filepath;
  ifstream input_file;

  std::string input;
public:
  InputFileIterator(std::string input_filepath_) :
    input_filepath(input_filepath_) {
    input_file(input_filepath);
  }
  InputFileIterator& operator++() {
    // Override this to provide your own custom handling.
    std::getline(input_file, input);
    return input;
  }
  InputFileIterator operator++(int) {
    operator++(); return input;
  }
  std::string get() {
    return input;
  }
};

typedef NetTestOptions std::map<std::string, std::string>;

class NetTest {
  
  std::string input_filepath;
  NetTestOptions options;
  FileReporter file_report;

public:
  NetTest(void);

  NetTest(std::string input_filepath);

  NetTest(std::string input_filepath, NetTestOptions options);

  InputFileIterator input_file();

  void geoip_lookup();

  void begin();

  void write_header();

  void end();

  virtual void main(std::string input);

}

}}}

#endif  // LIBIGHT_OONI_NETTEST_HPP
