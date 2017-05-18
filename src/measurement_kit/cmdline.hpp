// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_MEASUREMENT_KIT_CMDLINE_HPP
#define SRC_MEASUREMENT_KIT_CMDLINE_HPP

#include <measurement_kit/nettests.hpp>

#include "portable/err.h"
#include "portable/getopt.h"
#include "portable/unistd.h"

using namespace mk;
using namespace mk::nettests;

#define MK_CMDLINE_SUBCOMMANDS                                                 \
    XX(captive_portal)                                                         \
    XX(dns_injection)                                                          \
    XX(http_header_field_manipulation)                                         \
    XX(http_invalid_request_line)                                              \
    XX(meek_fronted_requests)                                                  \
    XX(multi_ndt)                                                              \
    XX(ndt)                                                                    \
    XX(tcp_connect)                                                            \
    XX(web_connectivity)

#define XX(_namespace_)                                                        \
    namespace _namespace_ {                                                    \
    int main(std::list<Callback<BaseTest &>> &initializers, int argc,          \
             char **argv);                                                     \
    }
MK_CMDLINE_SUBCOMMANDS
#undef XX

BaseTest &common_init(std::list<Callback<BaseTest &>>, BaseTest &);
BaseTest &ndt_init(std::list<Callback<BaseTest &>>, BaseTest &);

class OptionSpec {
  public:
    OptionSpec(int short_name, const char *long_name, bool requires_argument,
               const char *argument_name, const char *description)
        : short_name(short_name), long_name(long_name),
          requires_argument(requires_argument), argument_name(argument_name),
          description(description) {}

    int short_name = 0;
    const char *long_name = nullptr;
    bool requires_argument = false;
    const char *argument_name = nullptr;
    const char *description = nullptr;
};

std::vector<option> as_long_options(const OptionSpec *);
std::string as_getopt_string(const OptionSpec *);
std::string as_available_options_string(const OptionSpec *);

#endif
