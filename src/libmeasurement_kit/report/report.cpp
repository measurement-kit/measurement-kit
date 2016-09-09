// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <cstring>
#include <measurement_kit/report.hpp>
#include "../common/utils.hpp"

namespace mk {
namespace report {

Report::Report() {
    memset(&test_start_time, 0, sizeof (test_start_time));
}

void Report::add_reporter(Var<BaseReporter> reporter) {
    reporters_.push_back(reporter);
}

void Report::fill_entry(Entry &entry) const {
    entry["test_name"] = test_name;
    entry["test_version"] = test_version;
    entry["test_start_time"] = *mk::timestamp(&test_start_time);
    // header["options"] = options;
    entry["probe_ip"] = probe_ip;
    entry["probe_asn"] = probe_asn;
    entry["probe_cc"] = probe_cc;
    entry["software_name"] = software_name;
    entry["software_version"] = software_version;
    entry["data_format_version"] = data_format_version;
}

Entry Report::get_dummy_entry() const {
    Entry entry;
    fill_entry(entry);
    return entry;
}

#define FMAP mk::fmap<Var<BaseReporter>, Continuation<Error>>

void Report::open(Callback<Error> callback) {
    mk::parallel(FMAP(reporters_, [=](Var<BaseReporter> r) {
        return r->open(*this);
    }), callback);
}

void Report::write_entry(Entry entry, Callback<Error> callback) {
    mk::parallel(FMAP(reporters_, [=](Var<BaseReporter> r) {
        return r->write_entry(entry);
    }), callback);
}

void Report::close(Callback<Error> callback) {
    mk::parallel(FMAP(reporters_, [](Var<BaseReporter> r) {
        return r->close();
    }), callback);
}

#undef FMAP // So long, and thanks for the fish

} // namespace report
} // namespace mk
