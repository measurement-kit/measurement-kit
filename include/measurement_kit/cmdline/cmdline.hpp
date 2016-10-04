// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_CMDLINE_MAIN_HPP
#define MEASUREMENT_KIT_CMDLINE_MAIN_HPP

namespace mk {
namespace cmdline {

// The main of specific modules:

namespace dns_injection {
int main(const char *progname, int argc, char **argv);
}

namespace dns_query {
int main(const char *progname, int argc, char **argv);
}

namespace http_invalid_request_line {
int main(const char *progname, int argc, char **argv);
}

namespace http_request {
int main(const char *progname, int argc, char **argv);
}

namespace mlabns {
int main(const char *progname, int argc, char **argv);
}

namespace ndt {
int main(const char *progname, int argc, char **argv);
}

namespace net_connect {
int main(const char *progname, int argc, char **argv);
}

namespace oonireport {
int main(const char *progname, int argc, char **argv);
}

namespace ooniresources {
int main(const char *progname, int argc, char **argv);
}

namespace tcp_connect {
int main(const char *progname, int argc, char **argv);
}

namespace web_connectivity {
int main(const char *progname, int argc, char **argv);
}

// The toplevel main():

int main(const char *progname, int argc, char **argv);

} // namespace cmdline
} // namespace mk
#endif
