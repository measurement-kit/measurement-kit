// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_REPORT_ENTRY_HPP
#define MEASUREMENT_KIT_REPORT_ENTRY_HPP

#include <yaml-cpp/yaml.h>
#include <ctime>

namespace measurement_kit {
namespace report {

class ReportEntry {
  public:
    YAML::Node report;

    ReportEntry() { report["input"] = ""; }

    ReportEntry(std::string input) { report["input"] = input; }

    template <typename Key> const YAML::Node operator[](const Key &key) const {
        return report[key];
    }
    template <typename Key> YAML::Node operator[](const Key &key) {
        return report[key];
    }

    std::string str() {
        std::stringstream output;
        output << "---" << std::endl;
        output << report << std::endl;
        output << "..." << std::endl;
        return output.str();
    }

    ~ReportEntry() {}
};

} // namespace report
} // namespace measurement_kit
#endif
