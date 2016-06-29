// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/common/utils.hpp"
#include "src/ooni/utils.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

void OoniTest::run_next_measurement(Callback<> cb) {
    logger->debug("net_test: running next measurement");
    std::string next_input;
    std::getline(*input_generator, next_input);
    if (input_generator->eof()) {
        logger->debug("net_test: reached end of input");
        cb();
        return;
    }
    if (!input_generator->good()) {
        logger->warn("net_test: I/O error reading input");
        // TODO: allow to pass error to callback
        cb();
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
        entry["input"] = next_input;
        entry["measurement_start_time"] =
            *mk::timestamp(&measurement_start_time);
        entry["test_runtime"] = mk::time_now() - start_time;

        logger->debug("net_test: tearing down");
        teardown(next_input);

        file_report.write_entry(entry);
        logger->debug("net_test: written entry");

        reactor->call_soon([=]() { run_next_measurement(cb); });
    });
}

void OoniTest::geoip_lookup(Callback<> cb) {
    probe_ip = "127.0.0.1";
    probe_asn = "AS0";
    probe_cc = "ZZ";
    ip_lookup(
        [=](Error err, std::string ip) {
            if (err) {
                logger->warn("ip_lookup() failed: error code: %d", err.code);
            } else {
                logger->info("probe ip: %s", ip.c_str());
                if (options.get("save_real_probe_ip", false)) {
                    logger->debug("saving user's real ip on user's request");
                    probe_ip = ip;
                }
                std::string country_p =
                    options.get("geoip_country_path", std::string{});
                std::string asn_p =
                    options.get("geoip_asn_path", std::string{});
                if (country_p == "" or asn_p == "") {
                    logger->warn("geoip files not configured; skipping");
                } else {
                    ErrorOr<nlohmann::json> res = geoip(ip, country_p, asn_p);
                    if (!!res) {
                        logger->debug("GeoIP result: %s", res->dump().c_str());
                        // Since `geoip()` sets defaults before querying, the
                        // following accesses of json should not fail unless for
                        // programmer error after refactoring. In that case,
                        // better to let the exception unwind than just print
                        // a warning, because the former is easier to notice
                        // and therefore fix during development
                        probe_asn = (*res)["asn"];
                        logger->info("probe_asn: %s", probe_asn.c_str());
                        probe_cc = (*res)["country_code"];
                        logger->info("probe_cc: %s", probe_cc.c_str());
                    }
                }
            }
            cb();
        },
        options, reactor, logger);
}

void OoniTest::open_report() {
    file_report.test_name = test_name;
    file_report.test_version = test_version;
    file_report.test_start_time = test_start_time;

    file_report.options = options;

    file_report.probe_ip = probe_ip;
    file_report.probe_cc = probe_cc;
    file_report.probe_asn = probe_asn;

    if (output_filepath == "") {
        output_filepath = generate_output_filepath();
    }
    file_report.filename = output_filepath;
    file_report.open();
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
        // If a file called this way already exists we increase the counter by 1.
        if (output_file.good()) {
            output_file.close();
            idx++;
            continue;
        }
        break;
    }
    return filename.str();
}

void OoniTest::begin(Callback<> cb) {
    mk::utc_time_now(&test_start_time);
    geoip_lookup([=]() {
        open_report();
        if (needs_input) {
            if (input_filepath == "") {
                // TODO: allow to pass error to cb()
                logger->warn("an input file is required");
                cb();
                return;
            }
            input_generator.reset(new std::ifstream(input_filepath));
            if (!input_generator->good()) {
                logger->warn("cannot read input file");
                cb();
                return;
            }
        } else {
            input_generator.reset(new std::istringstream("\n"));
        }
        run_next_measurement(cb);
    });
}

void OoniTest::end(Callback<> cb) {
    file_report.close();
    collector::submit_report(
        output_filepath,
        options.get("collector_base_url", collector::default_collector_url()),
        [=](Error) { cb(); }, options, reactor, logger);
}

} // namespace ooni
} // namespace mk
