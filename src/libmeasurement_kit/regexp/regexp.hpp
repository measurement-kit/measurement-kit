// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_REGEXP_REGEXP_HPP
#define SRC_LIBMEASUREMENT_KIT_REGEXP_REGEXP_HPP

#include <string>

namespace mk {
namespace regexp {

bool valid_country_code(const std::string &input);

bool valid_airport_iata_code(const std::string &input);

bool lowercase_letters_only(const std::string &input);

std::string replace_probe_cc(std::string &&input, const std::string &cc);

bool valid_nettest_name(const std::string &input);

bool valid_nettest_version(const std::string &input);

bool valid_probe_asn(const std::string &input);

bool valid_test_start_time(const std::string &input);

std::string html_extract_title(const std::string &input);

bool private_ipv4(const std::string &input);

std::string ubuntu_xml_extract_ip(const std::string &input);

} // namespace regexp
} // namespace mk
#endif // SRC_LIBMEASUREMENT_KIT_REGEXP_REGEXP_HPP
