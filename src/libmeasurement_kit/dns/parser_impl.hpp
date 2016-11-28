/*
 * Part of measurement-kit <https://measurement-kit.github.io/>.
 * Measurement-kit is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 * =========================================================================
 * Portions Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_PARSER_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_PARSER_IMPL_HPP
// Based on: https://github.com/c-ares/c-ares/blob/master/adig.c

#include <cassert>

#include <ares.h>
#include <ares_dns.h> /* XXX make sure we check for this */

#include "../dns/ares_map_failure.hpp"
#include "../dns/parser.hpp"

namespace mk {
namespace dns {

/*-
 * Let this be the system memory:
 *
 *     0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
 *   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *   |   |   |   |   |   |   |[0]|[1]|[2]|[3]|[4]|[5]|   |   |   |   |
 *   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *                             ^                      ^
 *                            abuf               abuf + alen
 *
 * The question is when the region [aptr, aptr + off) is contained
 * within the regione [abuf, abuf + alen). This happens when:
 *
 *      aptr >= abuf and aptr + off <= abuf + alen
 *
 * It follows naturally that `max(off) = alen`. So we can write:
 *
 *      aptr >= abuf and off <= alen and aptr + off <= abuf + alen
 *
 * Still `aptr + off` can overflow (e.g. with `aptr = 11` and
 * `off = 6`); to prevent that we can write:
 *
 *      aptr >= abuf and off <= alen and aptr - abuf <= alen - off
 *
 * Where both the left and the right side of the last comparison are
 * guaranteed to be always positive by the previous checks.
 *
 * The cast is necessary because otherwise we would be comparing integers
 * of potentially different signedness. And also the cast is valid because
 * we have excluded cases where the difference is negative.
 */
static inline bool valid_mem_region(const unsigned char *aptr, size_t off,
                                    const unsigned char *abuf, size_t alen) {
    // Implementation note: checks reorganized to see through coverage
    // that we're taking all the possible branches
    if (not (aptr >= abuf)) {
        return false;
    }
    if (not (off <= alen)) {
        return false;
    }
    using ull = unsigned long long;
    if (not ((ull)(aptr - abuf) <= (ull)(alen - off))) {
        return false;
    }
    return true;
}

#define VALIDATE_APTR(aptr, abuf, alen)                                        \
    assert(valid_mem_region(aptr, 0, abuf, alen))

/**
 * Parse the initial header of a packet.
 * @param aptr Current pointer within packet.
 * @param abuf Start of the packet.
 * @param alen Size of the packet.
 * @param message Message where to write header fields.
 * @param logger Logger to use.
 * @return Updated aptr on success, error on failure.
 */
static inline ErrorOr<const unsigned char *>
parse_header(const unsigned char *aptr, const unsigned char *abuf, size_t alen,
             Var<Message> message, Var<Logger> logger) {
    if (!valid_mem_region(aptr, NS_HFIXEDSZ, abuf, alen)) {
        logger->warn("dns: packet too short: no space for header");
        return NoSpaceForHeaderError();
    }
    message->qid = DNS_HEADER_QID(aptr);
    message->qr = DNS_HEADER_QR(aptr);
    message->opcode = DNS_HEADER_OPCODE(aptr);
    message->aa = DNS_HEADER_AA(aptr);
    message->tc = DNS_HEADER_TC(aptr);
    message->rd = DNS_HEADER_RD(aptr);
    message->ra = DNS_HEADER_RA(aptr);
    message->z = DNS_HEADER_Z(aptr);
    message->rcode = DNS_HEADER_RCODE(aptr);
    message->qdcount = DNS_HEADER_QDCOUNT(aptr);
    message->ancount = DNS_HEADER_ANCOUNT(aptr);
    message->nscount = DNS_HEADER_NSCOUNT(aptr);
    message->arcount = DNS_HEADER_ARCOUNT(aptr);
    aptr += NS_HFIXEDSZ;
    logger->debug("dns: successfully parsed header");
    return aptr;
}

/**
 * Parse the next question record in the message.
 * @param aptr Current pointer within packet.
 * @param abuf Start of the packet.
 * @param alen Size of the packet.
 * @param query Object where to write the question record.
 * @param logger Logger to use.
 * @return Updated aptr on success, error on failure.
 */
template <MK_MOCK(ares_expand_name), MK_MOCK(ares_free_string)>
ErrorOr<const unsigned char *>
parse_question_impl(const unsigned char *aptr, const unsigned char *abuf,
                    size_t alen, Query &query, Var<Logger> logger) {
    VALIDATE_APTR(aptr, abuf, alen);
    char *name = nullptr;
    long len = 0;
    int status = ares_expand_name(aptr, abuf, alen, &name, &len);
    if (status != ARES_SUCCESS) {
        Error err = ares_map_failure(status);
        logger->warn("dns: in question: cannot expand name: %s",
                     err.explain().c_str());
        return err;
    }
    aptr += len;
    query.name = name;
    ares_free_string(name);
    if (!valid_mem_region(aptr, NS_QFIXEDSZ, abuf, alen)) {
        logger->warn("dns: packet too short: no space for query record");
        return NoSpaceForQueryError();
    }
    query.type = DNS_QUESTION_TYPE(aptr);
    query.qclass = DNS_QUESTION_CLASS(aptr);
    aptr += NS_QFIXEDSZ;
    logger->debug("dns: successfully parsed question record");
    return aptr;
}

/**
 * Parse the next resource record in the message.
 * @param aptr Current pointer within packet.
 * @param abuf Start of the packet.
 * @param alen Size of the packet.
 * @param answer Object where to write the resource record.
 * @param logger Logger to use.
 * @return Updated aptr on success, error on failure.
 */
template <MK_MOCK_SUFFIX(ares_expand_name, FOR_NAME),
          MK_MOCK_SUFFIX(ares_expand_name, FOR_PTR), MK_MOCK(inet_ntop),
          MK_MOCK(ares_free_string)>
ErrorOr<const unsigned char *> parse_rr_impl(
        const unsigned char *aptr, const unsigned char *abuf, size_t alen,
        Answer &answer, Var<Logger> logger) {
    int status;
    long len;
    char *name;
    VALIDATE_APTR(aptr, abuf, alen);
    status = ares_expand_name_FOR_NAME(aptr, abuf, alen, &name, &len);
    if (status != ARES_SUCCESS) {
        Error err = ares_map_failure(status);
        logger->warn("dns: in RR: cannot expand name: %s",
                     err.explain().c_str());
        return err;
    }
    aptr += len;
    answer.name = name;
    ares_free_string(name);
    if (!valid_mem_region(aptr, NS_RRFIXEDSZ, abuf, alen)) {
        logger->warn("dns: packet too short: no space for RR header");
        return NoSpaceForResourceRecordHeaderError();
    }
    answer.type = DNS_RR_TYPE(aptr);
    answer.aclass = DNS_RR_CLASS(aptr);
    answer.ttl = DNS_RR_TTL(aptr);
    answer.dlen = DNS_RR_LEN(aptr);
    aptr += NS_RRFIXEDSZ;
    if (!valid_mem_region(aptr, answer.dlen, abuf, alen)) {
        logger->warn("dns: packet too short: no space for RR");
        return NoSpaceForResourceRecordError();
    }

    /* Process the RR data.  Don't touch aptr. */
    char addr[46] = {};
    switch (answer.type) {
    case MK_DNS_TYPE_CNAME:
    case MK_DNS_TYPE_NS:
    case MK_DNS_TYPE_PTR:
        status = ares_expand_name_FOR_PTR(aptr, abuf, alen, &name, &len);
        if (status != ARES_SUCCESS) {
            Error err = ares_map_failure(status);
            logger->warn("dns: in PTR: cannot expand name: %s",
                         err.explain().c_str());
            return err;
        }
        answer.hostname = name;
        ares_free_string(name);
        break;
    case MK_DNS_TYPE_A:
        if (answer.dlen != 4) {
            return InvalidRecordLengthError();
        }
        if (inet_ntop(AF_INET, aptr, addr, sizeof(addr)) == nullptr) {
            return InetNtopError();
        }
        answer.ipv4 = addr;
        break;
    case MK_DNS_TYPE_AAAA:
        if (answer.dlen != 16) {
            return InvalidRecordLengthError();
        }
        if (inet_ntop(AF_INET6, aptr, addr, sizeof(addr)) == nullptr) {
            return InetNtopError();
        }
        answer.ipv6 = addr;
        break;
    default:
        logger->warn("dns: unsupported record type: %d", (int)answer.type);
        break;
    }

    aptr += answer.dlen;
    return aptr;
}

template <MK_MOCK(parse_header), MK_MOCK(parse_question),
          MK_MOCK_SUFFIX(parse_rr, ANSWERS),
          MK_MOCK_SUFFIX(parse_rr, AUTHORITY),
          MK_MOCK_SUFFIX(parse_rr, ADDITIONAL)>
Error parse_into_impl(Var<Message> message, std::vector<uint8_t> packet,
                      Var<Logger> logger) {
    const unsigned char *abuf = packet.data();
    const unsigned char *aptr = abuf;
    size_t alen = packet.size();
    ErrorOr<const unsigned char *> maybe_aptr;

    maybe_aptr = parse_header(aptr, abuf, alen, message, logger);
    if (!maybe_aptr) {
        // Warning message already printed by parse_header()
        return maybe_aptr.as_error();
    }
    aptr = *maybe_aptr;

#define XX(_counter_, _Type_, _Func_, _Dest_)                                  \
    do {                                                                       \
        for (uint16_t i = 0; i < _counter_; ++i) {                             \
            _Type_ _elem_{};                                                   \
            maybe_aptr = _Func_(aptr, abuf, alen, _elem_, logger);             \
            if (!maybe_aptr) {                                                 \
                /* Warning message already printed by _Func_() */              \
                return maybe_aptr.as_error();                                  \
            }                                                                  \
            aptr = *maybe_aptr;                                                \
            _Dest_.push_back(_elem_);                                          \
        }                                                                      \
    } while (0)

    XX(message->qdcount, Query, parse_question, message->queries);
    XX(message->ancount, Answer, parse_rr_ANSWERS, message->answers);
    XX(message->nscount, Answer, parse_rr_AUTHORITY, message->authorities);
    XX(message->arcount, Answer, parse_rr_ADDITIONAL, message->additionals);

#undef XX

    return NoError();
}

} // namespace dns
} // namespace mk
#endif
