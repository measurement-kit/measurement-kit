#ifndef LIBIGHT_REPORTER_BASE_HPP
#define LIBIGHT_REPORTER_BASE_HPP

#include "yaml-cpp/yaml.h"
#include "report/entry.hpp"

class ReporterBase {
  bool closed = false;
  bool openned = false;

  const std::string software_name = "ight";
  const std::string software_version = "0.0.1";
  const std::string data_format_version = "0.1";
  
public:
  const std::string test_name;
  const std::string test_version;
  const std::string probe_ip;

  std::string probe_asn;
  std::string probe_cc;

  const time_t start_time;

  const std::map<std::string, std::string> options;

  ReporterBase(const std::string& test_name, const std::string& test_version,
               const time_t& start_time, const std::string& probe_ip,
               const std::map<std::string, std::string>& options);

  ReporterBase(const ReporterBase& that) : test_name(that.test_name),
    test_version(that.test_version), probe_ip(that.probe_ip),
    probe_asn(that.probe_asn), probe_cc(that.probe_cc),
    start_time(that.start_time), options(that.options) {
    closed = that.closed;
    openned = that.openned;
  };

  ~ReporterBase() {};

  std::string getHeader();

  virtual void open();

  virtual void writeEntry(ReportEntry& entry);
  
  virtual void close();
};

#endif /* BASE_HPP */
