/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_REPORT_BASE_HPP
#define IGHT_REPORT_BASE_HPP

#include <yaml-cpp/yaml.h>
#include <ight/report/entry.hpp>

namespace ight {
namespace report {
namespace base {

using namespace ight::report::entry;

class ReporterBase {
  bool closed = false;
  bool openned = false;

  const std::string software_name = "ight";
  const std::string software_version = "0.0.1";
  const std::string data_format_version = "0.1";
  
public:
  std::string test_name;
  std::string test_version;
  std::string probe_ip;

  std::string probe_asn;
  std::string probe_cc;

  time_t start_time;

  std::map<std::string, std::string> options;

  ReporterBase(void) {};

  ~ReporterBase() {};

  std::string getHeader();

  virtual void open();

  virtual void writeEntry(ReportEntry& entry);
  
  virtual void close();
};

}}}
#endif
