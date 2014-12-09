#include "ooni/nettest.hpp"

using namespace ight::ooni::nettest;

NetTest::NetTest(void) : input_filepath(nullptr), options()
{
  
}

NetTest::NetTest(std::string input_filepath_) :
  input_filepath(input_filepath_), options()
{
}

NetTest::NetTest(std::string input_filepath_, ight::common::Settings options_) :
  input_filepath(input_filepath_), options(options_)
{
}

InputFileIterator
NetTest::input_file()
{
  return InputFileIterator(input_filepath);
}

void
NetTest::geoip_lookup()
{


void
NetTest::run_next_measurement(std::function<void()>&& cb)
{
  if (input==input.end()) {
    cb();
    return;
  }
  main(*input, options, [&](ReportEntry&& entry) {
      file_report.writeEntry(entry);
      input++;
      run_next_measurement(&cb);
  }); 
}

void
NetTest::begin(std::function<void()>&& cb)
{
  geoip_lookup();
  file_report = FileReporter(test_name, test_version, start_time, probe_ip,
      options, filename);
  write_header();
  if (input_filepath != nullptr){
    input = input_file();
    run_next_measurement(&cb);
  } else {
    main(options, [&](ReportEntry&& entry) {
      file_report.writeEntry(entry);
      cb();
    });
  }
}

void
NetTest::write_header()
{
  file_report.open();
}

void
NetTest::end(std::function<void()>&& cb)
{
  file_report.close();
  cb();
}
