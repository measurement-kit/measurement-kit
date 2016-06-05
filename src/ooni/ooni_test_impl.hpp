// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_OONI_TEST_HPP
#define SRC_OONI_OONI_TEST_HPP

#include <ctime>                                // for gmtime, strftime, time
#include <fstream>                              // for string, char_traits
#include <functional>                           // for function, __base
#include <measurement_kit/common.hpp>
#include <measurement_kit/ooni.hpp>
#include <measurement_kit/report.hpp>
#include <string>                               // for allocator, operator+
#include <type_traits>                          // for move
#include "src/common/utils.hpp"                 // for utc_time_now
#include "src/ooni/input_file_generator.hpp"    // for InputFileGenerator
#include "src/ooni/input_generator.hpp"         // for InputGenerator
#include "src/ooni/utils.hpp"
#include <sys/stat.h>

namespace mk {
namespace ooni {

class OoniTestImpl : public mk::NetTest {
    std::string input_filepath;
    report::FileReporter file_report;
    std::string report_filename;

    struct tm test_start_time;

    void run_next_measurement(const std::function<void()> &&cb) {
        logger->debug("net_test: running next measurement");
        input->next(
            [=](std::string next_input) {
                logger->debug("net_test: creating entry");

                struct tm measurement_start_time;
                double start_time;

                mk::utc_time_now(&measurement_start_time);
                start_time = mk::time_now();

                logger->debug("net_test: calling setup");
                setup();

                logger->debug("net_test: running with input %s",
                             next_input.c_str());

                main(next_input, options, [=](report::Entry test_keys) {
                    report::Entry entry;
                    entry["test_keys"] = test_keys;
                    entry["input"] = next_input;
                    entry["measurement_start_time"] = *mk::timestamp(&measurement_start_time);
                    entry["test_runtime"] = mk::time_now() - start_time;

                    logger->debug("net_test: tearing down");
                    teardown();

                    file_report.write_entry(entry);
                    logger->debug("net_test: written entry");

                    logger->debug("net_test: increased");
                    run_next_measurement(std::move(cb));
                });
            },
            [=]() {
                logger->debug("net_test: reached end of input");
                cb();
            });
    }

    void geoip_lookup(Callback<> cb) {
        probe_ip = "127.0.0.1";
        probe_asn = "AS0";
        probe_cc = "ZZ";
        ip_lookup([=](Error err, std::string ip) {
            if (err) {
                logger->warn("ip_lookup() failed: error code: %d", err.code);
            } else {
                logger->info("probe ip: %s", ip.c_str());
                if (options.get("save_real_probe_ip", false)) {
                    logger->debug("saving user's real ip on user's request");
                    probe_ip = ip;
                }
                std::string country_p = options.get("geoip_country_path",
                                                    std::string{});
                std::string asn_p = options.get("geoip_asn_path",
                                                std::string{});
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
        });
    }

    void open_report() {
        file_report.test_name = test_name;
        file_report.test_version = test_version;
        file_report.test_start_time = test_start_time;

        file_report.options = options;
        file_report.filename = get_report_filename();

        file_report.probe_ip = probe_ip;
        file_report.probe_cc = probe_cc;
        file_report.probe_asn = probe_asn;

        file_report.open();
    }

    std::string generate_report_filename() {
        std::string filename;
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%FT%H%M%SZ",
                 &test_start_time);
        std::string iso_8601_date(buffer);
        filename = "report-" + test_name + "-";
        filename += iso_8601_date + ".json";
        return filename;
    }

  protected:
    virtual void setup(std::string) {}
    virtual void setup() {}

    virtual void teardown(std::string) {}
    virtual void teardown() {}

    virtual void main(Settings, std::function<void(report::Entry)> &&cb) {
        reactor->call_later(1.25, [cb]() {
            report::Entry entry;
            cb(entry);
        });
    }

    virtual void main(std::string, Settings,
                      std::function<void(report::Entry)> &&cb) {
        reactor->call_later(1.25, [cb]() {
            report::Entry entry;
            cb(entry);
        });
    }

    void validate_input_filepath() {
        if (input_filepath == "") {
            throw InputFileRequired("An input file is required!");
        }

        struct stat buffer;
        if (stat(input_filepath.c_str(), &buffer) != 0) {
            throw InputFileDoesNotExist(input_filepath + " does not exist");
        }
    }

  public:
    report::Entry entry;
    InputGenerator *input = nullptr;

    std::string test_name;
    std::string test_version;

    std::string probe_ip = "127.0.0.1";
    std::string probe_asn = "AS0";
    std::string probe_cc = "ZZ";

    OoniTestImpl() : OoniTestImpl("", Settings()) {}

    virtual ~OoniTestImpl() {
        delete input;
        input = nullptr;
    }

    OoniTestImpl(std::string input_filepath_)
        : OoniTestImpl(input_filepath_, Settings()) {}

    OoniTestImpl(std::string input_filepath_, Settings options_)
        : NetTest(options_), input_filepath(input_filepath_),
        test_name("net_test"), test_version("0.0.1") {
            mk::utc_time_now(&test_start_time);
        }

    void set_report_filename(std::string s) { report_filename = s; }

    std::string get_report_filename() {
        if (report_filename == "") {
            report_filename = generate_report_filename();
        }
        return report_filename;
    }

    InputGenerator *input_generator() {
        return new InputFileGenerator(input_filepath, logger);
    }

    void begin(Callback<> cb) override {
        geoip_lookup([=]() {
            open_report();
            if (input_filepath != "") {
                logger->debug("net_test: found input file");
                if (input != nullptr) {
                    delete input;
                    input = nullptr;
                }
                input = input_generator();
                run_next_measurement(std::move(cb));
            } else {
                logger->debug("net_test: no input file");
                report::Entry entry;
                entry["input"] = "";
                logger->debug("net_test: calling setup");
                setup();
                main(options, [=](report::Entry entry) {
                    logger->debug("net_test: tearing down");
                    teardown();
                    file_report.write_entry(entry);
                    logger->debug("net_test: written entry");
                    logger->debug("net_test: reached end of input");
                    cb();
                });
            }
        });
    }

    void end(std::function<void()> cb) override {
        file_report.close();
        // Note: here we make the reasonable assumption that the owner of this
        // instance would keep it safe until the final callback is fired
        collector::submit_report(
            get_report_filename(),
            options.get("collector_base_url",
                        collector::default_collector_url()),
            [=](Error) { cb(); }, options, reactor, logger);
    }

    void set_reactor(Var<Reactor> p) { reactor = p; }
};

} // namespace ooni
} // namespace mk
#endif
