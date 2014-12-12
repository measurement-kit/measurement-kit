#ifndef LIBIGHT_FILE_REPORTER_HPP
# define LIBIGHT_FILE_REPORTER_HPP

#include <iostream>
#include <fstream>

#include "report/base.hpp"

class FileReporter : public ReporterBase {
  std::ofstream file;
public:
  std::string filename;
  void open();
  void writeEntry(ReportEntry& entry);
  void close();
};

#endif
