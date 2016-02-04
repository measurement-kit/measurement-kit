// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_OONI_TEST_HPP
#define SRC_OONI_OONI_TEST_HPP

#include <yaml-cpp/node/impl.h>                 // for Node::Node
#include <ctime>                                // for gmtime, strftime, time
#include <fstream>                              // for string, char_traits
#include <functional>                           // for function, __base
#include <measurement_kit/common/logger.hpp>    // for Logger
#include <measurement_kit/common/net_test.hpp>  // for NetTest
#include <measurement_kit/common/settings.hpp>  // for Settings
#include <string>                               // for allocator, operator+
#include <type_traits>                          // for move
#include "src/common/delayed_call.hpp"          // for DelayedCall
#include "src/common/libs_impl.hpp"             // for Libs
#include "src/ooni/input_file_generator.hpp"    // for InputFileGenerator
#include "src/ooni/input_generator.hpp"         // for InputGenerator
#include "src/report/entry.hpp"                 // for Entry
#include "src/report/file_reporter.hpp"         // for FileReporter

namespace mk {
namespace ooni {

class OoniTest : public mk::NetTest {
    std::string input_filepath;
    report::FileReporter file_report;
    std::string report_filename;

    DelayedCall delayed_call;

    void run_next_measurement(const std::function<void()> &&cb) {
        logger.debug("net_test: running next measurement");
        input->next(
            [=](std::string next_input) {
                logger.debug("net_test: creating entry");
                entry = report::Entry(next_input);
                logger.debug("net_test: calling setup");
                setup();
                logger.debug("net_test: running with input %s",
                             next_input.c_str());
                main(next_input, options, [=](report::Entry entry) {
                    logger.debug("net_test: tearing down");
                    teardown();
                    file_report.writeEntry(entry);
                    logger.debug("net_test: written entry");
                    logger.debug("net_test: increased");
                    run_next_measurement(std::move(cb));
                });
            },
            [=]() {
                logger.debug("net_test: reached end of input");
                cb();
            });
    }

    void geoip_lookup() {
        probe_ip = "127.0.0.1";
        probe_asn = "AS0";
        probe_cc = "ZZ";
    }

    void write_header() {
        file_report.test_name = test_name;
        file_report.test_version = test_version;
        time(&file_report.start_time);

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
                 gmtime(&file_report.start_time));
        std::string iso_8601_date(buffer);
        filename = "report-" + file_report.test_name + "-";
        filename += iso_8601_date + ".yamloo";
        return filename;
    }

  protected:
    Libs *libs = Libs::global();

    virtual void setup(std::string) {}
    virtual void setup() {}

    virtual void teardown(std::string) {}
    virtual void teardown() {}

    virtual void main(Settings, std::function<void(report::Entry)> &&cb) {
        delayed_call = DelayedCall(1.25, [=](void) {
            report::Entry entry;
            cb(entry);
        });
    }

    virtual void main(std::string, Settings,
                      std::function<void(report::Entry)> &&cb) {
        delayed_call = DelayedCall(1.25, [=](void) {
            report::Entry entry;
            cb(entry);
        });
    }

  public:
    report::Entry entry;
    Settings options;
    InputGenerator *input = nullptr;

    std::string test_name;
    std::string test_version;

    std::string probe_ip = "127.0.0.1";
    std::string probe_asn = "AS0";
    std::string probe_cc = "ZZ";

    OoniTest() : OoniTest("", Settings()) {}

    virtual ~OoniTest() {
        delete input;
        input = nullptr;
    }

    OoniTest(OoniTest &) = delete;
    OoniTest &operator=(OoniTest &) = delete;
    OoniTest(OoniTest &&) = default;
    OoniTest &operator=(OoniTest &&) = default;

    OoniTest(std::string input_filepath_)
        : OoniTest(input_filepath_, Settings()) {}

    OoniTest(std::string input_filepath_, Settings options_)
        : input_filepath(input_filepath_), options(options_),
          test_name("net_test"), test_version("0.0.1") {}

    void set_report_filename(std::string s) { report_filename = s; }

    std::string get_report_filename() {
        if (report_filename == "") {
            report_filename = generate_report_filename();
        }
        return report_filename;
    }

    InputGenerator *input_generator() {
        return new InputFileGenerator(input_filepath, &logger);
    }

    /*!
     * \brief Start iterating over the input.
     * \param cb Callback called when we are done.
     */
    virtual void begin(std::function<void()> cb) override {
        geoip_lookup();
        write_header();
        if (input_filepath != "") {
            logger.debug("net_test: found input file");
            if (input != nullptr) {
                delete input;
                input = nullptr;
            }
            input = input_generator();
            run_next_measurement(std::move(cb));
        } else {
            logger.debug("net_test: no input file");
            entry = report::Entry();
            logger.debug("net_test: calling setup");
            setup();
            main(options, [=](report::Entry entry) {
                logger.debug("net_test: tearing down");
                teardown();
                file_report.writeEntry(entry);
                logger.debug("net_test: written entry");
                logger.debug("net_test: reached end of input");
                cb();
            });
        }
    }

    /*!
     * \brief Make sure that the report is written.
     * \param cb Callback called when the report is written.
     */
    virtual void end(std::function<void()> cb) override {
        file_report.close();
        cb();
    }
};

} // namespace ooni
} // namespace mk
#endif
