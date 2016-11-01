// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include "../ooni/utils.hpp"

namespace mk {
namespace ooni {

void OoniTest::run_next_measurement(size_t thread_id, Callback<Error> cb,
                                    size_t num_entries,
                                    Var<size_t> current_entry) {
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

    double prog = 0.0;
    if (num_entries > 0) {
        prog = *current_entry / (double)num_entries;
    }
    *current_entry += 1;
    logger->log(MK_LOG_INFO|MK_LOG_JSON,
        "{\"progress\": %f, \"type\": \"progress\"}", prog);

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
                logger->warn("cannot write entry");
                // Note: continue running measurements in this case because a
                // transient collector failure MUST NOT stop running tests
            } else {
                logger->debug("net_test: written entry");
            }
            reactor->call_soon([=]() {
                run_next_measurement(thread_id, cb, num_entries, current_entry);
            });
        });
    });
}

void OoniTest::geoip_lookup(Callback<> cb) {

    // This is to ensure that when calling multiple times geoip_lookup we
    // always reset the probe_ip, probe_asn and probe_cc values.
    probe_ip = "127.0.0.1";
    probe_asn = "AS0";
    probe_cc = "ZZ";

    auto save_ip = options.get("save_real_probe_ip", false);
    auto save_asn = options.get("save_real_probe_asn", true);
    auto save_cc = options.get("save_real_probe_cc", true);

    // This code block allows the caller to override probe variables
    if (save_ip and options.find("probe_ip") != options.end()) {
        probe_ip = options.at("probe_ip");
        save_ip = false; // We already have it, don't look it up and save it
    }
    if (save_asn and options.find("probe_asn") != options.end()) {
        probe_asn = options.at("probe_asn");
        save_asn = false; // Ditto
    }
    if (save_cc and options.find("probe_cc") != options.end()) {
        probe_cc = options.at("probe_cc");
        save_cc = false; // Ditto
    }

    // No need to perform further lookups if we don't need to save anything
    if (not save_ip and not save_asn and not save_cc) {
        cb();
        return;
    }

    ip_lookup(
        [=](Error err, std::string ip) {
            if (err) {
                logger->warn("ip_lookup() failed: error code: %d", err.code);
                cb();
                return;
            }
            logger->info("probe ip: %s", ip.c_str());
            if (save_ip) {
                logger->debug("saving user's real ip on user's request");
                probe_ip = ip;
            }

            auto country_path = options.get("geoip_country_path",
                                            std::string{});
            if (save_cc and country_path != "") {
                try {
                    probe_cc = *GeoipCache::global()
                       ->resolve_country_code(country_path, ip, logger);
                } catch (const Error &err) {
                    logger->warn("cannot lookup country code: %s",
                                 err.explain().c_str());
                }
            } else if (country_path == "") {
                logger->warn("geoip_country_path is not set");
            }

            auto asn_path = options.get("geoip_asn_path", std::string{});
            if (save_asn and asn_path != "") {
                try {
                    probe_asn = *GeoipCache::global()
                        ->resolve_asn(asn_path, ip, logger);
                } catch (const Error &err) {
                    logger->warn("cannot lookup asn: %s",
                                 err.explain().c_str());
                }
            } else if (asn_path == "") {
                logger->warn("geoip_asn_path is not set");
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
    if (!options.get("no_file_report", false)) {
        report.add_reporter(FileReporter::make(output_filepath));
    }
    if (!options.get("no_collector", false)) {
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
                size_t num_entries = 0;
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

                    // Count the number of entries
                    std::string next_input;
                    while ((std::getline(*input_generator, next_input))) {
                        num_entries += 1;
                    }
                    if (!input_generator->eof()) {
                        logger->warn("cannot read input file");
                        cb(FileIoError());
                        return;
                    }
                    // See http://stackoverflow.com/questions/5750485
                    //  and http://stackoverflow.com/questions/28331017
                    input_generator->clear();
                    input_generator->seekg(0);

                } else {
                    input_generator.reset(new std::istringstream("\n"));
                    num_entries = 1;
                }


                // Run `parallelism` measurements in parallel
                Var<size_t> current_entry(new size_t(0));
                mk::parallel(
                    mk::fmap<size_t, Continuation<Error>>(
                        mk::range<size_t>(options.get("parallelism", 3)),
                        [=](size_t thread_id) {
                            return [=](Callback<Error> cb) {
                                run_next_measurement(thread_id, cb, num_entries,
                                                     current_entry);
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
