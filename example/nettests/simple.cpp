// Public domain 2017, Simone Basso <bassosimone@gmail.com.

#include <measurement_kit/ext.hpp>       // Import nlohmann::json
#include <measurement_kit/nettests.hpp>  // Import mk::nettests

#include <stdio.h>

/*
 * Run multi-ndt test in a single threaded app.
 *
 * Compile with:
 *
 *   c++ -Wall -o simple simple.cpp -lmeasurement_kit
 */
int main(void) {

    mk::nettests::MultiNdtTest()

        // By default measurement-kit only prints warnings
        .set_verbosity(MK_LOG_INFO)

        // Lambda called to notify us about how the test is making progress
        .on_progress([](double percent, const char *message) {
            printf("[%.1f%%] - %s\n", percent * 100.0, message);
        })

        // Lambda called when events occur. Here we process only download
        // speed updates emitted during the multi-ndt test.
        //
        // Note: in general `nlohmann::json` throws on error but MK
        // guarantees that on_event() is robust to exceptions.
        .on_event([](const char *s) {
            nlohmann::json doc = nlohmann::json::parse(s);
            if (doc["type"] != "download-speed") {
                return;
            }
            double elapsed = doc["elapsed"][0];
            std::string elapsed_unit = doc["elapsed"][1];
            double speed = doc["speed"][0];
            std::string speed_unit = doc["speed"][1];
            printf("%8.2f %s %10.2f %s\n", elapsed, elapsed_unit.c_str(),
                   speed, speed_unit.c_str());
        })

        // Lambda called at the end of the test with final results. Here
        // we parse it to print a summary on stdout.
        //
        // Also in this case we don't care about exceptions because
        // MK suppresses exceptions occurring in this lambda.
        .on_entry([](std::string s) {
            nlohmann::json doc = nlohmann::json::parse(s);
            auto simple = doc["test_keys"]["simple"];
            printf("\nTest summary\n");
            printf("------------\n");
            std::string fastest = simple["fastest_test"];
            double download = simple["download"];
            double ping = simple["ping"];
            printf("Fastest test: %s\n", fastest.c_str());
            printf("Download speed: %.2f kbit/s\n", download);
            printf("Ping: %.2f ms\n", ping);
            printf("\n");
        })

        // Lambda called to process log messages. The level is a bitmask
        // defined in the <measurement_kit/common/log.hpp> header.
        .on_log([](int level, const char *message) {
            fprintf(stderr, "<%d> %s\n", level, message);
        })

        // Tell MK where to find files useful to identify the country code
        // and the ISP autonomous system number. You should probably have
        // them installed under /usr/local/ in a reall program.
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")

        // Will become the default in MK v0.5.0. Better to use this flag
        // to avoid the need to discover the DNS server on mobile.
        .set_options("dns/engine", "system")

        // This instruction actually runs the test configured by all
        // the code lines above. This style of running a test is such
        // that the test runs in the current thread.
        .run();

    return 0;
}
