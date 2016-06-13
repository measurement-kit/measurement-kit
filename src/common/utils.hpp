// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_COMMON_UTILS_HPP
#define SRC_COMMON_UTILS_HPP

#include <event2/util.h>
#include <list>
#include <measurement_kit/common.hpp>
#include <stddef.h>
#include <string>
#include <unistd.h>

struct sockaddr_storage;
struct timeval;

namespace mk {

// Forward declarations
template<typename T> class Var;
class Logger;
class Settings;

int storage_init(sockaddr_storage *, socklen_t *, const char *, const char *,
                 const char *);
int storage_init(sockaddr_storage *, socklen_t *, int, const char *,
                 const char *);
int storage_init(sockaddr_storage *, socklen_t *, int, const char *, int);
evutil_socket_t socket_create(int, int, int);

std::string random_within_charset(std::string charset, size_t length);
std::string random_printable(size_t length);
std::string random_str(size_t length);
std::string random_str_uppercase(size_t length);

std::string unreverse_ipv6(std::string s);
std::string unreverse_ipv4(std::string s);

std::list<std::string> split(std::string s, std::string pattern = "\\s+");

void dump_settings(Settings &s, std::string prefix, Var<Logger> logger);

} // namespace mk
#endif
