// Public domain 2017, Simone Basso <bassosimone@gmail.com.

#define MK_CXX14_TRACE

#include <measurement_kit/common.hpp>
#include <measurement_kit/nettests.hpp> // Import mk::nettests

#include <stdio.h>

#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

/*
 * Run multi-ndt test in a multi threaded app. This example uses more
 * features from the `mk::nettests` namespace than the simple one.
 *
 * Compile with:
 *
 *   c++ -Wall -o multithread multithread.cpp -lmeasurement_kit
 */
int main(void) {

    /*
     * Lay out basic framwork for simulating a multi threaded app (e.g. a iOS
     * or Android, a macOS app using dispatch_async).
     *
     * All the lambdas in this section use `[&]` because they refer to
     * objects having the same life cycle of the main() function.
     */

    std::atomic<bool> again{true};
    std::mutex mutex;
    std::deque<std::function<void()>> queue;

    auto schedule = [&](std::function<void()> func) {
        std::unique_lock<std::mutex> lock{mutex};
        queue.push_back(std::move(func));
    };

    auto run = [&]() {
        while (again) {
            std::function<void()> func;
            {
                std::unique_lock<std::mutex> lock{mutex};
                if (queue.size() > 0) {
                    func = std::move(queue.front());
                    queue.pop_front();
                }
            }
            if (!func) {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
                continue;
            }
            try {
                func();
            } catch (const std::exception &exc) {
                fprintf(stderr, "warn: unhandled exception: %s\n", exc.what());
                /* SUPPRESS */;
            }
        }
    };

    /*
     * Schedule the test and report all events back to the main thread
     * to simulate what would happen on iOS or Android.
     */

    schedule([&]() {
        mk::nettests::MultiNdtTest()

            // By default measurement-kit only prints warnings
            .set_verbosity(MK_LOG_INFO)

            // Do not write on disk the results of the test
            .set_option("no_file_report", "1")

            /*
             * Lambda called to notify about test progress.
             *
             * The lambda uses `[&]` because it only refers to objects in
             * main() scope. However, note how internally instead we
             * need to do the following:
             *
             * 1. make `message` persistent through `safe` (`message` is
             *    a pointer inside of a temporary buffer)
             *
             * 2. copy `safe` and `percent` such that they are still alive
             *    when the inner lambda is executed by the main thread
             *
             * 3. synchronize with the main() thread (this is done implicitly
             *    by the `schedule()` primitive we defined)
             *
             * 4. care about exceptions (again `run()` does that)
             *
             * In general, these are the four points to keep in mind when
             * running in a lambda called back by measurement-kit.
             */
            .on_progress([&](double percent, const char *message) {
                std::string safe = message;
                schedule([percent, safe]() {
                    printf("[%.1f%%] - %s\n", percent * 100.0, safe.c_str());
                });
            })

            // Lambda called when events occur. Here we process only download
            // speed updates emitted during the multi-ndt test.
            //
            // In case `Json::parse()` throws an exception, no
            // worries because MK suppress exceptions in the on_event lambda.
            //
            // See the above four points to keep in mind.
            .on_event([&](const char *s) {
                mk::Json doc = mk::Json::parse(s);
                if (doc["type"] != "download-speed") {
                    return;
                }
                schedule([doc]() {
                    double elapsed = doc["elapsed"][0];
                    std::string elapsed_unit = doc["elapsed"][1];
                    double speed = doc["speed"][0];
                    std::string speed_unit = doc["speed"][1];
                    printf("%8.2f %s %10.2f %s\n", elapsed,
                           elapsed_unit.c_str(), speed, speed_unit.c_str());
                });
            })

            // Lambda called at the end of the test with final results. Here
            // we parse it to print a summary on stdout.
            //
            // See the above four points to keep in mind.
            .on_entry([&](std::string s) {
                schedule([s]() {
                    mk::Json doc = mk::Json::parse(s);
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
                });
            })

            // Lambda called to process log messages. The level is a bitmask
            // defined in the <measurement_kit/common/log.hpp> header.
            //
            // See the above four points to keep in mind.
            .on_log([&](int level, const char *message) {
                std::string safe = message;
                schedule([level, safe]() {
                    fprintf(stderr, "<%d> %s\n", level, safe.c_str());
                });
            })

            // Tell MK where to find files useful to identify the country code
            // and the ISP autonomous system number. You should probably have
            // them installed under /usr/local/ in a reall program.
            .set_option("geoip_country_path", "GeoIP.dat")
            .set_option("geoip_asn_path", "GeoIPASNum.dat")

            // Will become the default in MK v0.5.0. Better to use this flag
            // to avoid the need to discover the DNS server on mobile.
            .set_option("dns/engine", "system")

            /*
             * This instruction starts the test in a background thread
             * and calls the callback passed as argument when done.
             *
             * See the above four points to keep in mind.
             */
            .start([&]() {
                again = false; /* Note: `again` is atomic */
            });
    });

    /*
     * Finally, start the main thread.
     */

    run();

    return 0;
}
