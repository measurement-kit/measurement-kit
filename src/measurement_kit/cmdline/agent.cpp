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

class Context {
  public:
    bool authenticated = false;
    Var<net::Buffer> buff{new net::Buffer};
    Var<Logger> logger;
    std::string passwd = "";
    Var<Reactor> reactor;
    Var<net::Transport> txp;
};

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
 * @param ctx Client ctx.
 * @param line The remainder of the line.
 */
static void do_auth(Var<Context> ctx, std::string &line) {
    if (line != ctx->passwd) {
        ctx->logger->warn("passwords do not match ('%s' and '%s')",
                          line.c_str(), ctx->passwd.c_str());
        ctx->authenticated = false;
        ctx->txp->write("ERR invalid-password\r\n{}\r\n");
        return;
    }
    ctx->logger->debug("passwords match; you are now authenticated");
    ctx->authenticated = true;
    ctx->txp->write("OK\r\n{}\r\n");
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
 * @param ctx Client context.
 * @param ln The remainder of the line.
 * @param resume Callback.
 */
static void do_web_connectivity(Var<Context> ctx, const std::string &ln,
                                Callback<> resume) {

    Settings options = {};
    std::string input = "";

    if (get_input_and_settings(ln, input, options) != NoError()) {
        ctx->txp->write("ERR bad-request\r\n{}\r\n");
        resume();
        return;
    }
    if (input == "") {
        ctx->txp->write("ERR bad-request\r\n{}\r\n");
        resume();
        return;
    }

    ooni::web_connectivity(input, options,
                           [=](Var<report::Entry> entry) {
                               ctx->txp->write("OK\r\n");
                               ctx->txp->write(entry->dump());
                               ctx->txp->write("\r\n");
                               resume();
                           },
                           ctx->reactor, ctx->logger);
}

/**
 * Processes the RUN command.
 * @param ctx Client ctx.
 * @param line The remainder of the line.
 * @param resume Callback.
 */
static void do_run(Var<Context> ctx, std::string &line, Callback<> resume) {
    ErrorOr<std::string> method = remove_next_token(line);
    if (!method) {
        ctx->txp->write("ERR bad-request\r\n{}\r\n");
        resume();
        return;
    }
    if (*method == "web_connectivity") {
        do_web_connectivity(ctx, line, resume);
        return;
    }
    /*
     * TODO add here more test helpers or more basic blocks
     */
    ctx->txp->write("ERR not-found\r\n{}\r\n");
    resume();
}

/**
 * Close connection with client.
 * @remark Necessary to remove a self reference that prevents closing txp.
 * @param ctx Client ctx.
 */
static void do_close_txp(Var<Context> ctx) {
    Var<net::Transport> txp = ctx->txp;
    ctx->txp = nullptr;
    txp->close([=]() { ctx->logger->debug("connection with client closed"); });
}

/**
 * Wait for more data coming from the client.
 * @param ctx Client ctx.
 * @param callback The callback.
 */
static void wait_for_more_data(Var<Context> ctx, Callback<> callback) {
    net::read(ctx->txp, ctx->buff,
              [=](Error error) {
                  if (error) {
                      ctx->logger->debug("error while reading: %s",
                                         error.explain().c_str());
                      do_close_txp(ctx);
                      return;
                  }
                  callback();
              },
              ctx->reactor);
}

/**
 * Serve requests coming from connected client.
 * @param ctx Client context.
 */
static void serve(Var<Context> ctx) {
    auto resume = [=]() { ctx->reactor->call_soon([=]() { serve(ctx); }); };

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
    ErrorOr<std::string> maybe_line = ctx->buff->readline(10 * 1024 * 1024);
    if (!maybe_line) {
        do_close_txp(ctx);
        return;
    }
    if (*maybe_line == "") {
        wait_for_more_data(ctx, resume);
        return;
    }
    static const std::regex re{"[\r\n]+$"};
    *maybe_line = std::regex_replace(*maybe_line, re, "");

    ErrorOr<std::string> cmd = remove_next_token(*maybe_line);
    if (!cmd) {
        ctx->logger->warn("invalid line: %s", maybe_line->c_str());
        ctx->txp->write("ERR bad-request\r\n{}\r\n");
        resume();
        return;
    }
    ctx->logger->debug("command: %s", cmd->c_str());

    if (*cmd == "AUTH") {
        do_auth(ctx, *maybe_line);
        resume();
        return;
    }

    if (ctx->authenticated) {
        if (*cmd == "RUN") {
            do_run(ctx, *maybe_line, resume);
            return;
        }
        // FALLTHROUGH
    }

    if (!ctx->authenticated) {
        ctx->txp->write("ERR not-authenticated\r\n{}\r\n");
    } else {
        ctx->txp->write("ERR not-found\r\n{}\r\n");
    }
    resume();
}

int main(const char *, int argc, char **argv) {
    /*
     * XXX: we must use the global reactor until listen4() is updated to
     * support also non global reactors.
     */
    Var<Reactor> reactor = Reactor::global();
    Var<Logger> logger = Logger::global();

    std::string passwd;
    int port = 9876;
    for (int ch; (ch = mkp_getopt(argc, argv, "p:v")) != -1;) {
        switch (ch) {
        case 'p':
            port = lexical_cast<int>(mkp_optarg);
            break;
        case 'v':
            logger->increase_verbosity();
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

    reactor->loop_with_initial_event([=]() {
        libevent::listen4("127.0.0.1", port, [=](bufferevent *bev) {
            logger->info("connection made from new client");
            Var<Context> ctx{new Context};
            ctx->txp = libevent::Connection::make(bev);
            ctx->logger = logger;
            ctx->reactor = reactor;
            ctx->passwd = passwd;
            serve(ctx);
        });
    });

    return 0;
}

} // namespace agent
} // namespace cmdline
} // namespace mk
