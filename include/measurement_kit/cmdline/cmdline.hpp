// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_CMDLINE_MAIN_HPP
#define MEASUREMENT_KIT_CMDLINE_MAIN_HPP

namespace mk {
namespace cmdline {

#define XX(_namespace_) \
namespace _namespace_ { \
int main(const char *progname, int argc, char **argv); \
}

// The main of specific modules:

XX(dns_injection)
XX(dns_query)
XX(http_invalid_request_line)
XX(http_request)
XX(mlabns)
XX(ndt)
XX(net_connect)
XX(oonireport)
XX(ooniresources)
XX(tcp_connect)
XX(web_connectivity)

// The toplevel main():

int main(const char *progname, int argc, char **argv);

#undef XX

} // namespace cmdline
} // namespace mk
#endif
