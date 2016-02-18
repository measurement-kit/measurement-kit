// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/maybe.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/net/buffer.hpp>
#include <measurement_kit/net/transport.hpp>
#include <stdlib.h>
#include <vector>
#include "src/tor.hpp"

namespace mk {

// Forward declaration
class Logger;

namespace tor {

static void
readline(Control ctrl, std::function<void(Error, std::string)> callback) {
    // First check whether we can pull a line from the already buffered data
    Maybe<std::string> maybe_line = ctrl.buffer->readline(1024);
    if (not maybe_line) {
        callback(maybe_line.as_error(), "");
        return;
    }
    std::string line = maybe_line.as_value();
    if (line != "") {
        // Avoid recursion if multiple lines are pulled in a row
        ctrl.poller->call_soon([callback, line]() {
            callback(NoError(), line);
        });
        return;
    }
    ctrl.transport.on_data([callback, ctrl](net::Buffer data) {
        *ctrl.buffer << data;
        Maybe<std::string> maybe_line = ctrl.buffer->readline(1024);
        if (not maybe_line) {
            ctrl.transport.on_error(nullptr);
            ctrl.transport.on_data(nullptr);
            callback(maybe_line.as_error(), "");
            return;
        }
        std::string line = maybe_line.as_value();
        if (line == "") {
            return; // We need to refill the buffer
        }
        ctrl.transport.on_data(nullptr);
        ctrl.transport.on_error(nullptr);
        callback(NoError(), line);
    });
    ctrl.transport.on_error([callback, ctrl](Error error) {
        ctrl.transport.on_error(nullptr);
        ctrl.transport.on_data(nullptr);
        callback(error, "");
    });
}

static void sendcommand(Control ctrl, std::string command,
                        std::function<void(Error)> cb) {
    ctrl.transport.on_error([ctrl, cb](Error error) {
        ctrl.transport.on_error(nullptr);
        ctrl.transport.on_flush(nullptr);
        cb(error);
    });
    ctrl.transport.on_flush([ctrl, cb]() {
        ctrl.transport.on_error(nullptr);
        ctrl.transport.on_flush(nullptr);
        cb(NoError());
    });
    ctrl.transport.send(command);
}

static void read_simple_response(Control ctrl,
                                 std::function<void(Error)> callback) {
    readline(ctrl, [callback](Error error, std::string line) {
        if (error) {
            callback(error);
            return;
        }
        if (line.size() < 4 or not isdigit(line[0]) or not isdigit(line[1]) or
            not isdigit(line[2]) or line[3] != ' ') {
            callback(GenericError());
            return;
        }
        int result = atoi(line.substr(0, 4).c_str());
        if (result != 250) {
            callback(GenericError());
            return;
        }
        callback(NoError());
    });
}

void authenticate(std::function<void(Error, Control)> callback,
                  std::string address, std::string port, Poller *poller,
                  Logger *logger) {
    poller->call_soon([callback, address, port, poller, logger]() {
        Control ctrl;
        Maybe<net::Transport> maybe_transport = net::connect(
            {
                {"port", port}, {"address", address},
            },
            logger, poller);
        if (!maybe_transport) {
            callback(maybe_transport.as_error(), ctrl);
            return;
        }
        ctrl.poller = poller;
        ctrl.transport = maybe_transport.as_value();
        ctrl.buffer.reset(new net::Buffer);
        sendcommand(ctrl, "AUTHENTICATE\r\n", [callback, ctrl](Error error) {
            if (error) {
                callback(error, ctrl);
                return;
            }
            read_simple_response(
                ctrl, [callback, ctrl](Error error) { callback(error, ctrl); });
        });
    });
}

void setconf_disable_network(Control ctrl, bool disable,
                             std::function<void(Error)> callback) {
    std::string command = "SETCONF DisableNetwork=";
    command += (disable) ? "1" : "0";
    command += "\r\n";
    sendcommand(ctrl, command, [callback, ctrl](Error error) {
        if (error) {
            callback(error);
            return;
        }
        read_simple_response(ctrl, callback);
    });
}

static std::vector<std::string> tokenize(std::string s) {
    std::vector<std::string> vector;
    std::string cur;
    for (char &c : s) {
        if (isspace(c)) {
            if (cur != "") {
                vector.push_back(cur);
                cur = "";
            }
        } else {
            cur += c;
        }
    }
    if (cur != "") vector.push_back(cur);
    return vector;
}

static Maybe<int> parse_bootstrap_progress(std::string s) {
    // E.g. NOTICE BOOTSTRAP PROGRESS=0 TAG=starting SUMMARY="Starting"
    std::vector<std::string> vec = tokenize(s);
    if (vec.size() < 3) {
        return Maybe<int>(GenericError(), 0);
    }
    if (vec[0] != "NOTICE") {
        return Maybe<int>(GenericError(), 0);
    }
    if (vec[1] != "BOOTSTRAP") {
        return Maybe<int>(GenericError(), 0);
    }
    if (vec[2].find("PROGRESS=") != 0) {
        return Maybe<int>(GenericError(), 0);
    }
    vec[2] = vec[2].substr(sizeof("PROGRESS=") - 1);
    if (vec[2].size() > 3) {
        return Maybe<int>(GenericError(), 0);
    }
    for (char &c : vec[2]) {
        if (!isdigit(c)) {
            return Maybe<int>(GenericError(), 0);
        }
    }
    return Maybe<int>(NoError(), atoi(vec[2].c_str()));
}

static void read_getinfo_status_bootstrap_phase_line(
    Control ctrl, std::function<void(Error, int)> callback) {
    readline(ctrl, [callback](Error error, std::string line) {
        if (error) {
            callback(error, 0);
            return;
        }
        if (line.find("250-status/bootstrap-phase=") != 0) {
            callback(GenericError(), 0);
            return;
        }
        line = line.substr(sizeof "250-status/bootstrap-phase=" - 1);
        Maybe<int> result = parse_bootstrap_progress(line);
        if (!result) {
            callback(result.as_error(), 0);
            return;
        }
        callback(NoError(), result.as_value());
    });
}

void getinfo_status_bootstrap_phase(Control ctrl,
                                    std::function<void(Error, int)> callback) {
    sendcommand(ctrl, "GETINFO status/bootstrap-phase\r\n",
                [callback, ctrl](Error error) {
                    if (error) {
                        callback(error, 0);
                        return;
                    }
                    read_getinfo_status_bootstrap_phase_line(
                        ctrl, [callback, ctrl](Error error, int phase) {
                            // XXX Here it would be better to distinguish
                            // between recoverable and non-recoverable errors
                            read_simple_response(
                                ctrl, [callback, error, phase](Error nested) {
                                    if (error) {
                                        callback(error, 0);
                                    } else if (nested) {
                                        callback(nested, 0);
                                    } else
                                        callback(NoError(), phase);
                                });
                        });
                });
}

} // namespace tor
} // namespace mk
