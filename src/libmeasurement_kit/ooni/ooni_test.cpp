// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include "../ooni/utils.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

void OoniTest::run_next_measurement(size_t index, Callback<Error> cb) {
    logger->debug("net_test: running next measurement");
    std::string next_input;
    std::getline(*input_generator, next_input);
    if (input_generator->eof()) {
        logger->debug("net_test: reached end of input");
        cb(NoError());
        return;
    }
    if (!input_generator->good()) {
        logger->warn("net_test: I/O error reading input");
        cb(FileIoError());
        return;
    }

    logger->debug("net_test: creating entry");
    struct tm measurement_start_time;
    double start_time;
    mk::utc_time_now(&measurement_start_time);
    start_time = mk::time_now();

    logger->debug("net_test: calling setup");
    setup(next_input);

    logger->debug("net_test: running with input %s", next_input.c_str());
    main(next_input, options, [=](report::Entry test_keys) {
        report::Entry entry;
        entry["test_keys"] = test_keys;
        entry["test_keys"]["client_resolver"] = resolver_ip;
        entry["input"] = next_input;
        entry["measurement_start_time"] =
            *mk::timestamp(&measurement_start_time);
        entry["test_runtime"] = mk::time_now() - start_time;

        logger->debug("net_test: tearing down");
        teardown(next_input);

        report.fill_entry(entry);
        if (entry_cb) {
            entry_cb(entry.dump());
        }
        report.write_entry(entry, [=](Error error) {
            if (error) {
                cb(error);
                return;
            }
            logger->debug("net_test: written entry");
            reactor->call_soon([=]() { run_next_measurement(index, cb); });
        });
    });
}

void OoniTest::geoip_lookup(Callback<> cb) {
    // This is to ensure that when calling multiple times geoip_lookup we
    // always reset the probe_ip, probe_asn and probe_cc values.
    probe_ip = "127.0.0.1";
    probe_asn = "AS0";
    probe_cc = "ZZ";
    ip_lookup(
        [=](Error err, std::string ip) {
            // TODO: refactor to use the if-err-then-cb-and-return style
            if (err) {
                logger->warn("ip_lookup() failed: error code: %d", err.code);
            } else {
                logger->info("probe ip: %s", ip.c_str());
                if (options.get("save_real_probe_ip", false)) {
                    logger->debug("saving user's real ip on user's request");
                    probe_ip = ip;
                }
                auto cntry_p = options.get("geoip_country_path", std::string{});
                auto asn_p = options.get("geoip_asn_path", std::string{});
                if (cntry_p == "" or asn_p == "") {
                    logger->warn("geoip files not configured; skipping");
                } else {
                    auto gi = IPLocation::memoized(cntry_p, asn_p, "", logger);
                    ErrorOr<std::string> s;
                    s = gi->resolve_asn(ip);
                    if (!!s) {
                        logger->info("probe_asn: %s", s->c_str());
                        if (options.get("save_real_probe_asn", true)) {
                            probe_asn = *s;
                        }
                    }
                    s = gi->resolve_country_code(ip);
                    if (!!s) {
                        logger->info("probe_cc: %s", s->c_str());
                        if (options.get("save_real_probe_cc", true)) {
                            probe_cc = *s;
                        }
                    }
                }
            }
            cb();
        },
        options, reactor, logger);
}

void OoniTest::open_report(Callback<Error> callback) {
    report.test_name = test_name;
    report.test_version = test_version;
    report.test_start_time = test_start_time;

    report.options = options;

    report.probe_ip = probe_ip;
    report.probe_cc = probe_cc;
    report.probe_asn = probe_asn;

    if (output_filepath == "") {
        output_filepath = generate_output_filepath();
    }
    report.add_reporter(FileReporter::make(output_filepath));
    if (options.find("no_collector") == options.end()) {
        report.add_reporter(OoniReporter::make(*this));
    }
    report.open(callback);
}

std::string OoniTest::generate_output_filepath() {
    int idx = 0;
    std::stringstream filename;
    while (true) {
        filename.str("");
        filename.clear();

        char timestamp[100];
        strftime(timestamp, sizeof(timestamp), "%FT%H%M%SZ", &test_start_time);
        filename << "report-" << test_name << "-";
        filename << timestamp << "-" << idx << ".json";

        std::ifstream output_file(filename.str().c_str());
        // If a file called this way already exists we increment the counter
        if (output_file.good()) {
            output_file.close();
            idx++;
            continue;
        }
        break;
    }
    return filename.str();
}

void OoniTest::begin(Callback<Error> cb) {
    if (begin_cb) {
        begin_cb();
    }
    mk::utc_time_now(&test_start_time);
    geoip_lookup([=]() {
        resolver_lookup([=](Error error, std::string resolver_ip_) {
            if (!error) {
                resolver_ip = resolver_ip_;
            } else {
                logger->debug("failed to lookup resolver ip");
            }
            open_report([=](Error error) {
                if (error) {
                    cb(error);
                    return;
                }
                if (needs_input) {
                    if (input_filepath == "") {
                        logger->warn("an input file is required");
                        cb(MissingRequiredInputFileError());
                        return;
                    }
                    input_generator.reset(new std::ifstream(input_filepath));
                    if (!input_generator->good()) {
                        logger->warn("cannot read input file");
                        cb(CannotOpenInputFileError());
                        return;
                    }
                } else {
                    input_generator.reset(new std::istringstream("\n"));
                }

                // Run `parallelism` measurements in parallel
                mk::parallel(
                    mk::fmap<size_t, Continuation<Error>>(
                        mk::range<size_t>(options.get("parallelism", 3)),
                        [=](size_t index) {
                            return [=](Callback<Error> cb) {
                                run_next_measurement(index, cb);
                            };
                        }),
                    cb);

            });
        }, options, reactor, logger);
    });
}

void OoniTest::end(Callback<Error> cb) {
    if (end_cb) {
        end_cb();
    }
    report.close(cb);
}

} // namespace ooni
} // namespace mk
