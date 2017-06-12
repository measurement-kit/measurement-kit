// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ext/json.hpp>

#include "../measurement_kit/cmdline.hpp"

BaseTest &common_init(std::list<Callback<BaseTest &>> il, BaseTest &test) {
    test
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .on_progress([](double progress, std::string msg) {
            printf("%.0f%%: %s\n", 100.0 * progress, msg.c_str());
        })
        .on_log([](uint32_t level, const char *message) {
            if (level <= MK_LOG_WARNING) {
                fprintf(stderr, "[!] ");
            } else if (level >= MK_LOG_DEBUG) {
                fprintf(stderr, "[D] ");
            }
            fprintf(stderr, "%s\n", message);
        });
    for (auto fn : il) {
        fn(test);
    }
    return test;
}

BaseTest &ndt_init(std::list<Callback<BaseTest &>> il, BaseTest &t) {
    return common_init(il, t).on_event([](const char *s) {
        // Note: `on_event()` filters all exceptions on our behalf
        nlohmann::json doc = nlohmann::json::parse(s);
        if (doc["type"] != "download-speed" && doc["type"] != "upload-speed") {
            return;
        }
        double elapsed = doc["elapsed"][0];
        std::string elapsed_unit = doc["elapsed"][1];
        double speed = doc["speed"][0];
        std::string speed_unit = doc["speed"][1];
        printf("%8.2f %s %10.2f %s\n", elapsed, elapsed_unit.c_str(), speed,
               speed_unit.c_str());
    });
}

std::vector<option> as_long_options(const OptionSpec *os) {
    std::vector<option> ret;
    for (auto sos = os; sos->short_name != 0; ++sos) {
        option op = {};
        op.name = sos->long_name;
        op.has_arg = (sos->requires_argument) ? required_argument : no_argument;
        op.val = sos->short_name;
        ret.push_back(op);
    }
    return ret;
}

std::string as_getopt_string(const OptionSpec *os) {
    /*
     * Note: the leading `+` tells GNU getopt() to avoid reordering
     * options, which allows us to have parse global options, followed
     * by a command, followed by specific options.
     */
    std::string ret = "+";
    for (auto sos = os; sos->short_name != 0; ++sos) {
        if (sos->short_name > 255) {
            continue;
        }
        ret += static_cast<char>(sos->short_name);
        if (sos->requires_argument) {
            ret += ":";
        }
    }
    return ret;
}

std::string as_available_options_string(const OptionSpec *os) {
    std::string ret = "Options:\n";
    for (auto sos = os; sos->short_name != 0; ++sos) {
        std::string x;
        if (sos->short_name <= 255) {
            x += "  -";
            x += static_cast<char>(sos->short_name);
            x += ", ";
        } else {
            x += "      ";
        }
        x += "--";
        x += sos->long_name;
        if (sos->argument_name != nullptr) {
            x += "=";
            x += sos->argument_name;
        }
        while (x.size() < 32) {
            x += " ";
        }
        x += sos->description;
        x += "\n";
        ret += x;
    }
    ret += "\n";
    return ret;
}
