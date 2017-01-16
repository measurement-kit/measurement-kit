// Public domain

#include <measurement_kit/ext.hpp>       // Import nlohmann::json
#include <measurement_kit/nettests.hpp>  // Import mk::nettests

#include <stdio.h>

int main(void) {

    mk::nettests::MultiNdtTest()

        .set_verbosity(MK_LOG_INFO)

        .on_progress([](double percent, const char *message) {
            printf("[%.1f%%] - %s\n", percent * 100.0, message);
        })

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

        .on_log([](int level, const char *message) {
            fprintf(stderr, "<%d> %s\n", level, message);
        })

        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")

        .run();

    return 0;
}
