// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#include "src/libmeasurement_kit/ndt/utils.hpp"

namespace mk {
namespace ndt {
namespace utils {

/*
 * Note: in the following function and other functions below we're going to
 * take advantage of the fact that `Entry` is a JSON type, thus we are going
 * to return, e.g., `nullptr` to indicate error and otherwise a double.
 *
 * This behavior is correct C++ because the constructor of `Entry` can take
 * in input any valid JSON (`null`, numbers, strings, lists, objects).
 */

report::Entry compute_ping(report::Entry &test_s2c, SharedPtr<Logger> logger) {

    try {
        // Note: do static cast to make sure it's convertible to a double
        return static_cast<double>(test_s2c["web100_data"]["MinRTT"]);
    } catch (const std::exception &w) {
        logger->warn("Cannot access Web100 data: %s", w.what());
        /* Fallthrough to next method of computing RTT */
    }

    std::vector<double> rtts;
    try {
        // XXX Needed to add temp variable because otherwise it did not compile
        std::vector<double> temp = test_s2c["connect_times"];
        rtts = temp;
    } catch (const std::exception &) {
        logger->warn("Cannot access connect times");
        /* Fallthrough to the following check that will fail */ ;
    }
    if (rtts.size() <= 0) {
        logger->warn("Did not find any reliable way to compute RTT");
        return nullptr; /* We cannot compute the RTT */
    }

    double sum = 0.0;
    for (auto &rtt: rtts) {
        sum += rtt * 1000.0 /* To milliseconds! */;
    }
    return sum / rtts.size();  /* Division by zero excluded above */
}

report::Entry compute_speed(report::Entry &sender_or_receiver_data,
                            const char *speed_type, SharedPtr<Logger> logger) {
    /*
     * This algorithm computes the speed in a way that is similar to the one
     * implemented by OOKLA, as documented here:
     *
     *    http://www.ookla.com/support/a21110547/what-is-the-test-flow-and-methodology-for-the-speedtest
     */
    try {
        std::vector<double> speeds;
        for (auto &x: sender_or_receiver_data) {
            speeds.push_back(x[1]);
        }
        std::sort(speeds.begin(), speeds.end());
        std::vector<double> good_speeds(
            // Note: going beyond vector limits would raise
            // a std::length_error exception
            speeds.begin() + 6, speeds.end() - 2
        );
        double sum = 0.0;
        for (auto &x : good_speeds) {
            sum += x;
        };
        if (good_speeds.size() <= 0) {
            logger->warn("The vector of good speeds is empty");
            return nullptr;
        }
        return sum / good_speeds.size();
    } catch (const std::exception &) {
        logger->warn("Cannot compute %s speed", speed_type);
        // FALLTHROUGH
    }
    return nullptr;
}

report::Entry compute_simple_stats(report::Entry &entry, SharedPtr<Logger> logger) {
    report::Entry test_s2c;
    report::Entry test_c2s;
    report::Entry simple_stats;

    try {
        /*
         * XXX The following code assumes there is just one entry, which is
         * really what usually happens. But there are ways to run the test
         * and have multiple S2C entries (e.g., pass both `-T download` and
         * `-T download_ext` to `./measurement_kit`).
         */
        if (entry["test_s2c"].size() <= 0) {
            throw std::runtime_error("missing entry");
        }
        test_s2c = entry["test_s2c"][0];
        simple_stats["download"] = compute_speed(test_s2c["receiver_data"],
                "download", logger);
        simple_stats["ping"] = compute_ping(test_s2c, logger);
    } catch (const std::exception &x) {
        logger->warn("cannot access entry[\"test_s2c\"][0]: %s", x.what());
        /* Cannot compute this stat */
    }

    try {
        /*
         * As of v0.4.0, we cannot have more than one entry here.
         */
        if (entry["test_c2s"].size() <= 0) {
            throw std::runtime_error("missing entry");
        }
        test_c2s = entry["test_c2s"][0];
        simple_stats["upload"] = compute_speed(test_c2s["sender_data"],
                "upload", logger);
    } catch (const std::exception &x) {
        logger->warn("cannot access entry[\"test_c2s\"][0]: %s", x.what());
        /* Cannot compute this stat */
    }
    return simple_stats;
}

report::Entry compute_advanced_stats(report::Entry &entry, SharedPtr<Logger>) {
    /*
     * Typically we have just one entry. But see above comment.
     */
    report::Entry test_s2c = entry["test_s2c"][0];
    report::Entry advanced_stats;

    // See: https://github.com/ndt-project/ndt/wiki/NDTTestMethodology#computed-variables

    double SndLimTimeRwin = test_s2c["web100_data"]["SndLimTimeRwin"];
    double SndLimTimeCwnd = test_s2c["web100_data"]["SndLimTimeCwnd"];
    double SndLimTimeSender = test_s2c["web100_data"]["SndLimTimeSender"];
    double TotalTestTime = SndLimTimeRwin + SndLimTimeCwnd + SndLimTimeSender;

    double CongestionSignals = test_s2c["web100_data"]["CongestionSignals"];
    double PktsOut = test_s2c["web100_data"]["PktsOut"];
    double PacketLoss = 0.0;
    if (PktsOut > 0.0) {
        PacketLoss = CongestionSignals / PktsOut;
    }

    double DupAcksIn = test_s2c["web100_data"]["DupAcksIn"];
    double AckPktsIn = test_s2c["web100_data"]["AckPktsIn"];
    double OutOfOrder = 0.0;
    if (AckPktsIn > 0.0) {
        OutOfOrder = DupAcksIn / AckPktsIn;
    }

    double SumRTT = test_s2c["web100_data"]["SumRTT"];
    double CountRTT = test_s2c["web100_data"]["CountRTT"];
    double AvgRTT = 0.0;
    if (CountRTT > 0.0) {
        AvgRTT = SumRTT / CountRTT;
    }

    double CongestionLimited = 0.0;
    if (TotalTestTime > 0.0) {
        CongestionLimited = SndLimTimeCwnd / TotalTestTime;
    }

    double ReceiverLimited = 0.0;
    if (TotalTestTime > 0.0) {
        ReceiverLimited = SndLimTimeRwin / TotalTestTime;
    }

    double SenderLimited = 0.0;
    if (TotalTestTime > 0.0) {
        SenderLimited = SndLimTimeSender / TotalTestTime;
    }

    advanced_stats["avg_rtt"] = AvgRTT;
    advanced_stats["mss"] = test_s2c["web100_data"]["CurMSS"];
    advanced_stats["max_rtt"] = test_s2c["web100_data"]["MaxRTT"];
    advanced_stats["min_rtt"] = test_s2c["web100_data"]["MinRTT"];
    advanced_stats["timeouts"] = test_s2c["web100_data"]["Timeouts"];
    advanced_stats["out_of_order"] = OutOfOrder;
    advanced_stats["packet_loss"] = PacketLoss;

    // These are not used in ooni's UI
    advanced_stats["congestion_limited"] = CongestionLimited;
    advanced_stats["fast_retran"] = test_s2c["web100_data"]["FastRetran"];
    advanced_stats["receiver_limited"] = ReceiverLimited;
    advanced_stats["sender_limited"] = SenderLimited;
    return advanced_stats;
}

} // namespace utils
} // namespace ndt
} // namespace mk
