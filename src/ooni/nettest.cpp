#include "ooni/nettest.hpp"

using namespace ight::ooni::nettest;

NetTest::NetTest(void) : input_filepath(nullptr), options()
{
  
}

NetTest::NetTest(std::string input_filepath_) :
  input_filepath(input_filepath_), options()
{
}

NetTest::NetTest(std::string input_filepath_, NetTestOptions options_) :
  input_filepath(input_filepath_), options(options_)
{
}

InputFileIterator
NetTest::input_file()
{
}

void
NetTest::geoip_lookup()
{
}

void
NetTest::begin()
{
  geoip_lookup();
  file_report = FileReporter(test_name, test_version, start_time, probe_ip,
      options, filename);
  write_header();
  if (input_filepath != nullptr){
    InputFileIterator input = input_file();
    for (;input.get()!=nullptr; input++) {
      ReportEntry entry;
      entry = main(input.get(), options);
      file_report.writeEntry(entry);
    }
  } else {
    main(options);
  }
  end();
}

void
NetTest::write_header()
{
  file_report.open();
}

void
NetTest::end()
{
  file_report.close();
}
