// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/regexp/regexp.hpp"

#include <regex>

namespace mk {
namespace regexp {

bool valid_country_code(const std::string &input) {
  return std::regex_match(input, std::regex{R"(^[A-Z]{2}$)"});
}

bool valid_airport_iata_code(const std::string &input) {
  return std::regex_match(input, std::regex{R"(^[a-z]{3}$)"});
}

bool lowercase_letters_only(const std::string &input) {
  return std::regex_match(input, std::regex{R"(^[a-z]+$)"});
}

std::string replace_probe_cc(std::string &&input, const std::string &cc) {
  return std::regex_replace(input, std::regex{R"(\$\{probe_cc\})"}, cc);
}

bool valid_nettest_name(const std::string &input) {
  return std::regex_match(input, std::regex{R"(^[A-Za-z0-9._-]+$)"});
}

bool valid_nettest_version(const std::string &input) {
  return std::regex_match(
      input,
      std::regex{R"(^v?[0-9]{1,8}\.[0-9]{1,8}\.[0-9]{1,8}[a-z0-9-+.]{0,32}$)"});
}

bool valid_probe_asn(const std::string &input) {
  return std::regex_match(input, std::regex{R"(^AS[0-9]+$)"});
}

bool valid_test_start_time(const std::string &input) {
  return std::regex_match(
      input,
      std::regex{R"(^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}$)"});
}

std::string html_extract_title(const std::string &input) {
  std::smatch match;
  std::regex re{R"(<title>([^<]{1,128})<\/title>)", std::regex::icase};
  if (std::regex_search(input, match, re) == false) {
    return "";
  }
  return (match.size() >= 2) ? match[1] : std::string{};
}

bool private_ipv4(const std::string &input) {
  const char *pattern = R"(^(?:(?:127\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})|(?:192\.168\.[0-9]{1,3}\.[0-9]{1,3})|(?:10\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})|(?:172\.1[6-9]\.[0-9]{1,3}\.[0-9]{1,3})|(?:172\.2[0-9]\.[0-9]{1,3}\.[0-9]{1,3})|(?:172\.3[0-1]\.[0-9]{1,3}\.[0-9]{1,3})|localhost)$)";
  return std::regex_match(input, std::regex{pattern});
}

std::string ubuntu_xml_extract_ip(const std::string &input) {
  std::smatch match;
  std::regex re{R"(<Ip>([0-9.]{7,15}|[A-Fa-f0-9:]{3,39})<\/Ip>)"};
  if (std::regex_search(input, match, re) == false) {
    return "";
  }
  return (match.size() >= 2) ? match[1] : std::string{};
}

} // namespace regexp
} // namespace mk
