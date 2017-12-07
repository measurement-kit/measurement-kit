// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_DNS_UTILS_HPP
#define PRIVATE_DNS_UTILS_HPP

#include <measurement_kit/common/aaa_base.h>
#include <measurement_kit/common/data_usage.hpp>
#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

// `estimate_data_usage()` estimates the data used by resolving hostnames.
static inline void estimate_data_usage(DataUsage &du, std::string query_name,
        std::vector<dns::Answer> answers, SharedPtr<Logger> logger) {
    // # Upload
    //
    // Approximate algorithm to compute DNS usage: for each query we have
    // the fixed header, plus the query, plus the length of the name. In an
    // experimental packet capture I determined that this estimate misses
    // just two bytes from the actual packet. Seems to be good enough.
    du.up += NS_HFIXEDSZ + NS_QFIXEDSZ + query_name.size();

    // # Download
    for (auto answer : answers) {
        if (answer.type == dns::MK_DNS_TYPE_A) {
            // We clearly downloaded an A response message
            du.down += NS_HFIXEDSZ + NS_QFIXEDSZ + query_name.size() +
                       NS_RRFIXEDSZ + NS_INADDRSZ;
        } else if (answer.type == dns::MK_DNS_TYPE_AAAA) {
            // We clearly downloaded an AAAA response message
            du.down += NS_HFIXEDSZ + NS_QFIXEDSZ + query_name.size() +
                       NS_RRFIXEDSZ + NS_IN6ADDRSZ;
        } else if (answer.type == dns::MK_DNS_TYPE_CNAME) {
            // We clearly downloaded a CNAME response message. This may be
            // inaccurate since the name could have been compressed.
            du.down += NS_HFIXEDSZ + NS_QFIXEDSZ + query_name.size() +
                       NS_RRFIXEDSZ + answer.hostname.size();
        } else {
            logger->warn("estimate_data_usage: unsupported DNS query");
        }
    }
}

} // namespace dns
} // namespace mk
#endif
