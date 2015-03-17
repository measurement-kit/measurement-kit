/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_REPORT_FILE_HPP
# define IGHT_REPORT_FILE_HPP

#include <iostream>
#include <fstream>

#include <ight/report/base.hpp>

namespace ight {
namespace report {
namespace file {

using namespace ight::report::base;

class FileReporter : public ReporterBase {
  std::ofstream file;
public:
  std::string filename;
  void open();
  void writeEntry(ReportEntry& entry);
  void close();
};

}}}
#endif
