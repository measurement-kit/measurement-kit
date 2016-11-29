// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ext.hpp>
#include <measurement_kit/ooni.hpp>

#include "../cmdline/cmdline.hpp"
/*
 * TODO: made these interfaces available through measurement-kit public API
 */
#include "../../libmeasurement_kit/libevent/connection.hpp"
#include "../../libmeasurement_kit/libevent/listen.hpp"

namespace mk {
namespace cmdline {
namespace agent {

/*
 * FIXME With this usage, anyone can see the password using `ps auxww`
 */
#define USAGE "usage: measurement_kit agent [-v] [-p port] passwd\n"

/**
 * Remove the next token from string.
 * @param s String to read token from, updated in output.
 * @return Next token or error.
 */
static ErrorOr<std::string> remove_next_token(std::string &s) {
    size_t pos = s.find_first_of(" ");
    if (pos == std::string::npos) {
        return GenericError();
    }
    std::string token = s.substr(0, pos);
    s = s.substr(pos + 1);
    return token;
}

/**
 * Processes the AUTH command.
 * @param txp The connected transport.
 * @param authenticated Whether we're authenticated.
 * @param line The remainder of the line.
 * @param passwd The password.
 */
static void do_auth(Var<net::Transport> txp, Var<bool> authenticated,
                    std::string &line, std::string passwd) {
    if (line != passwd) {
        mk::warn("passwords do not match ('%s' and '%s')", line.c_str(),
                 passwd.c_str());
        *authenticated = false;
        txp->write("ERR unauthorized\r\n{}\r\n");
        return;
    }
    mk::debug("passwords match; you are now authenticated");
    *authenticated = true;
    txp->write("OK\r\n{}\r\n");
}

/**
 * Get input and settings from the remainder of the line.
 * @param line The remainder of the line.
 * @param input Where input will be copied. The input is optional and on
 *              missing `input` key, an empty string will be returned.
 * @param options Where settings will be copied.
 * @returns NoError() on success, otherwise an error.
 */
static Error get_input_and_settings(const std::string &line, std::string &input,
                                    Settings &options) {
    nlohmann::json root;

    try {
        root = nlohmann::json::parse(line);
    } catch (const std::exception &) {
        return GenericError();
    }

    if (root.find("input") != root.end()) {
        try {
            input = root["input"].get<std::string>();
        } catch (const std::exception &) {
            return GenericError();
        }
    } else {
        input = "";
    }

    auto root_settings = root["settings"];
    for (auto it = root_settings.begin(); it != root_settings.end(); ++it) {
        std::string key = "";
        std::string value = "";
        try {
            key = it.key();
            value = it.value();
        } catch (const std::exception &) {
            return GenericError();
        }
        options[key] = value;
    }

    return NoError();
}

/**
 * RUNs the web_connectivity test.
 * @param txp The connected transport.
 * @param line The remainder of the line.
 */
static void do_web_connectivity(Var<net::Transport> txp,
                                const std::string &line) {

    Settings options = {};
    std::string input = "";

    if (get_input_and_settings(line, input, options) != NoError()) {
        txp->write("ERR bad_request\r\n{}\r\n");
        return;
    }
    if (input == "") {
        txp->write("ERR bad_request\r\n{}\r\n");
        return;
    }

    /*
     * TODO: allow to run this in a shared, background reactor. It is easy
     * to do that, but I prefer to wait until it's clear we are going to use
     * this approach to move forward with integrate MK into ooni-probe.
     */
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        mk::debug("entering into nettest loop");
        ooni::web_connectivity(input, options,
                               [=](Var<report::Entry> entry) {
                                   txp->write("OK\r\n");
                                   txp->write(entry->dump());
                                   txp->write("\r\n");
                                   mk::debug("breaking out of nettest loop");
                                   reactor->break_loop();
                               },
                               reactor);
    });

    mk::debug("out of loop; resuming REPL...");
}

/**
 * Processes the RUN command.
 * @param txp The connected transport.
 * @param authenticated Whether we're authenticated.
 * @param line The remainder of the line.
 */
static void do_run(Var<net::Transport> txp, std::string &line) {
    ErrorOr<std::string> method = remove_next_token(line);
    if (!method) {
        txp->write("ERR bad_request\r\n{}\r\n");
        return;
    }
    if (*method == "web_connectivity") {
        do_web_connectivity(txp, line);
        return;
    }
    /*
     * TODO add here more test helpers or more basic blocks
     */
    txp->write("ERR not_found\r\n{}\r\n");
}

int main(const char *, int argc, char **argv) {
    std::string passwd;
    int port = 9876;
    for (int ch; (ch = mkp_getopt(argc, argv, "p:v")) != -1;) {
        switch (ch) {
        case 'p':
            port = lexical_cast<int>(mkp_optarg);
            break;
        case 'v':
            increase_verbosity();
            break;
        default:
            fprintf(stderr, "%s", USAGE);
            exit(1);
        }
    }
    argc -= mkp_optind, argv += mkp_optind;
    if (argc != 1) {
        fprintf(stderr, "%s", USAGE);
        exit(1);
    }
    passwd = argv[0];

    loop_with_initial_event([=]() {
        libevent::listen4("127.0.0.1", port, [=](bufferevent *bev) {
            Var<net::Transport> txp = libevent::Connection::make(bev);
            Var<net::Buffer> buff = net::Buffer::make();
            Var<bool> authenticated{new bool{false}};
            txp->on_error([=](Error err) {
                mk::warn("Connection error: %s", err.explain().c_str());
                txp->close([=]() { mk::debug("Connection lost"); });
            });
            txp->on_data([=](net::Buffer data) {
                *buff << data;
                for (;;) {

                    /*
                     * TODO Wish list for readline():
                     *
                     * 1. allow to read arbitrary long lines
                     *
                     * 2. use specific error rather than "" to signal
                     *    that you can need to read more from socket
                     *
                     * 3. (after 2) strip trailing \r\n
                     */
                    ErrorOr<std::string> maybe_line =
                        buff->readline(10 * 1024 * 1024);
                    if (!maybe_line) {
                        txp->close([=]() { mk::debug("Connection lost"); });
                        return;
                    }
                    if (*maybe_line == "") {
                        break; /* Try again after reading more */
                    }
                    static const std::regex re{"[\r\n]+$"};
                    *maybe_line = std::regex_replace(*maybe_line, re, "");

                    ErrorOr<std::string> cmd = remove_next_token(*maybe_line);
                    if (!cmd) {
                        mk::warn("invalid line");
                        continue;
                    }
                    mk::debug("command: %s", cmd->c_str());
                    if (*cmd == "AUTH") {
                        do_auth(txp, authenticated, *maybe_line, passwd);
                    } else if (!*authenticated) {
                        txp->write("ERR unauthorized\r\n{}\r\n");
                    } else if (*cmd == "RUN") {
                        do_run(txp, *maybe_line);
                    } else {
                        txp->write("ERR not_found\r\n{}\r\n");
                    }
                }
            });
        });
    });

    return 0;
}

} // namespace agent
} // namespace cmdline
} // namespace mk
