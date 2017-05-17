// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"
#include <measurement_kit/ext/json.hpp>
#include <measurement_kit/ndt.hpp>

namespace multi_ndt {

#define USAGE                                                                  \
    "usage: measurement_kit [options] multi_ndt [-u] [-m metro]\n"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    MultiNdtTest test;

    for (int ch; (ch = getopt(argc, argv, "m:u")) != -1;) {
        switch (ch) {
        case 'm':
            test.set_options("mlabns/policy", "metro");
            test.set_options("mlabns/metro", optarg);
            break;
        case 'u':
            // By default only the download phase is performed
            test.set_options("single_test_suite",
                    MK_NDT_DOWNLOAD | MK_NDT_UPLOAD);
            break;
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
            /* NOTREACHED */
        }
    }
    argc -= optind, argv += optind;
    if (argc != 0) {
        fprintf(stderr, "%s\n", USAGE);
        exit(1);
        /* NOTREACHED */
    }

    ndt_init(initializers, test.on_entry([](std::string s) {
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
        printf("\nAdvanced info (from single stream test)\n");
        printf("---------------------------------------\n");
        auto adv = doc["test_keys"]["advanced"];
        double AvgRTT = adv["avg_rtt"];
        double MinRTT = adv["min_rtt"];
        double MaxRTT = adv["max_rtt"];
        printf("RTT (avg/min/max): %.1f/%.1f/%.1f ms\n", AvgRTT, MinRTT,
               MaxRTT);
        double CongestionLimited = adv["congestion_limited"];
        double ReceiverLimited = adv["receiver_limited"];
        double SenderLimited = adv["sender_limited"];
        printf("Limited (congestion/receiver/sender): %.2f/%.2f/%.2f\n",
               CongestionLimited, ReceiverLimited, SenderLimited);
        double OutOfOrder = adv["out_of_order"];
        printf("OutOfOrder: %lf (%.3lf%%)\n", OutOfOrder, OutOfOrder * 100.0);
        unsigned long MSS = adv["mss"];
        printf("MSS: %lu byte\n", MSS);
        unsigned long FastRetran = adv["fast_retran"];
        unsigned long Timeouts = adv["timeouts"];
        printf("Congestion: FastRetran: %lu; Timeo: %lu\n", FastRetran,
               Timeouts);
        double PacketLoss = adv["packet_loss"];
        printf("Loss: %lf (%.3lf%%)\n", PacketLoss, PacketLoss * 100.0);
        printf("\n");
    })).run();

    return 0;
}

} // namespace multi_ndt
