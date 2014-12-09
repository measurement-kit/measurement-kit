#include "ooni/nettest.hpp"
#include <ctime>

using namespace ight::ooni::nettest;


NetTest::NetTest(std::string input_filepath_, ight::common::Settings options_) :
  input_filepath(input_filepath_), options(options_)
{
  const std::string report_file = "example_test_report.yaml";
  const std::string test_name = "light-meter";
  const std::string test_version = "0.0.1";
  const std::string probe_ip = "127.0.0.1";

  time_t start_time = time(0);
  std::map<std::string, std::string> options;
  options["opt1"] = "value1";
  options["opt2"] = "value2";

  file_report = FileReporter(test_name, test_version, start_time, probe_ip,
                             options, report_file);
}

NetTest::NetTest(void) : NetTest::NetTest(nullptr, nullptr)
{

}

NetTest::NetTest(std::string input_filepath_) : NetTest::NetTest(input_filepath_, nullptr)
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

}

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
      run_next_measurement(cb);
  }); 
}

void
NetTest::begin(std::function<void()>&& cb)
{
  geoip_lookup();
  write_header();
  if (input_filepath != nullptr){
    input = input_file();
    run_next_measurement(cb);
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
