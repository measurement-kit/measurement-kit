/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_REPORT_ENTRY_HPP
#define IGHT_REPORT_ENTRY_HPP

#include <ctime>
#include "yaml-cpp/yaml.h"

namespace ight {
namespace report {
namespace entry {

class ReportEntry {
public:
  YAML::Node report;

  ReportEntry() {
    report["input"] = "";
  };

  ReportEntry(std::string input) {
    report["input"] = input;
  };
  
  template <typename Key>
  const YAML::Node operator[](const Key& key) const {
    return report[key];
  }
  template <typename Key>
  YAML::Node operator[](const Key& key) {
    return report[key];
  }

  std::string str() {
    std::stringstream output;
    output << "---" << std::endl;
    output << report << std::endl;
    output << "..." << std::endl;
    return output.str();
  }

  ~ReportEntry() {};
};

}}}
#endif
