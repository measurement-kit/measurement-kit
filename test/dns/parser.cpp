// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/dns/parser_impl.hpp"

using namespace mk;

TEST_CASE("valid_mem_region() works as expected") {
#define XX(yy) (const unsigned char *)(yy)

    SECTION("For aptr < abuf") {
        REQUIRE(!dns::valid_mem_region(XX(0x00), 10, XX(0x01), 100));
    }
    SECTION("For off > alen") {
        REQUIRE(!dns::valid_mem_region(XX(0x00), 10, XX(0x00), 9));
    }
    SECTION("For aptr - abuf > alen - off") {
        REQUIRE(!dns::valid_mem_region(XX(0x0a), 8, XX(0x00), 10));
    }
    SECTION("For a valid case") {
        REQUIRE(dns::valid_mem_region(XX(0x06), 4, XX(0x00), 10));
    }

#undef XX
}

TEST_CASE("parse_header() works as expected") {

    // clang-format off
    const unsigned char pkt[] = {
        /*-snip--*/ 0x0c, 0x02, 0x81, 0x80, 0x00, 0x01, // ........
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x77, // .......w
        0x77, 0x77, 0x06, 0x67, 0x6f, 0x6f, 0x67, 0x6c, // ww.googl
        0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, // e.com...
        0x00, 0x01, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, // ........
        0x00, 0x00, 0x01, 0x06, 0x00, 0x04, 0xd8, 0x3a, // .......:
        0xd2, 0xe4                                      // .. */
    };
    // clang-format on

    Var<dns::Message> message{new dns::Message};
    ErrorOr<const unsigned char *> mebbe;

    SECTION("When the packet is short") {
        mebbe = dns::parse_header(pkt, pkt, 4, message, Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code == dns::NoSpaceForHeaderError().code);
    }

    SECTION("In the common case") {
        mebbe = dns::parse_header(pkt, pkt, sizeof(pkt) / sizeof(pkt[0]),
                                  message, Logger::global());
        REQUIRE(!!mebbe);
        REQUIRE(*mebbe == pkt + NS_HFIXEDSZ);
        REQUIRE(message->qid == 0x0c02);
        REQUIRE(message->qr);
        REQUIRE(message->opcode == 0);
        REQUIRE(!message->aa);
        REQUIRE(!message->tc);
        REQUIRE(message->rd);
        REQUIRE(message->ra);
        REQUIRE(!message->z);
        REQUIRE(message->rcode == 0);
        REQUIRE(message->qdcount == 1);
        REQUIRE(message->ancount == 1);
        REQUIRE(message->nscount == 0);
        REQUIRE(message->arcount == 0);
    }
}

TEST_CASE("map_ares_failure() works as expected") {

    SECTION("For mapped error codes") {
        REQUIRE(dns::map_ares_failure(ARES_SUCCESS).code == NoError().code);
        REQUIRE(dns::map_ares_failure(ARES_EBADNAME).code ==
                dns::MalformedEncodedDomainNameError().code);
        REQUIRE(dns::map_ares_failure(ARES_ENOMEM).code ==
                OutOfMemoryError().code);
    }

    SECTION("For non-mapped error codes") {
        REQUIRE(dns::map_ares_failure(ARES_ETIMEOUT).code ==
                GenericError().code);
    }
}

static int ares_expand_name_fail(unsigned const char *, unsigned const char *,
                                 int, char **, long *) {
    return ARES_EBADNAME;
}

static bool free_string_called = false;
static void ares_free_string_hook(void *s) {
    free_string_called = true;
    ares_free_string(s);
}

TEST_CASE("parse_question() works as expected") {

    // clang-format off
    const unsigned char pkt[] = {
        /*-snip--------------------------*/ 0x03, 0x77, // .......w
        0x77, 0x77, 0x06, 0x67, 0x6f, 0x6f, 0x67, 0x6c, // ww.googl
        0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, // e.com...
        0x00, 0x01, /*-snip--------------------------*/ // ........
    };
    // clang-format on

    dns::Query query;
    ErrorOr<const unsigned char *> mebbe;

    SECTION("In case of ares_expand_name() failure") {
        mebbe = dns::parse_question_impl<ares_expand_name_fail>(
            pkt, pkt, sizeof(pkt) / sizeof(pkt[0]), query, Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code ==
                dns::MalformedEncodedDomainNameError().code);
    }

    SECTION("In case the packet is too short") {
        mebbe = dns::parse_question(pkt, pkt, 16 /* short read */, query,
                                    Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code == dns::NoSpaceForQueryError().code);
    }

    SECTION("In case of success") {
        free_string_called = false;
        mebbe =
            dns::parse_question_impl<ares_expand_name, ares_free_string_hook>(
                pkt, pkt, sizeof(pkt) / sizeof(pkt[0]), query,
                Logger::global());
        REQUIRE(!!mebbe);
        REQUIRE(*mebbe == pkt + sizeof(pkt) / sizeof(pkt[0]));
        REQUIRE(free_string_called);
        REQUIRE(query.name == "www.google.com");
        REQUIRE(query.type == dns::MK_DNS_TYPE_A);
        REQUIRE(query.qclass == dns::MK_DNS_CLASS_IN);
    }
}

static const char *inet_ntop_fail(int, const void *, char *, socklen_t) {
    return nullptr;
}

TEST_CASE("parse_rr() works as expected") {

    // clang-format off

    static const unsigned char rr_a[] = {
        /*-snip--*/ 0x0c, 0x02, 0x81, 0x80, 0x00, 0x01, /* ........ */
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x77, /* .......w */
        0x77, 0x77, 0x06, 0x67, 0x6f, 0x6f, 0x67, 0x6c, /* ww.googl */
        0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, /* e.com... */
        0x00, 0x01, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, /* ........ */
        0x00, 0x00, 0x01, 0x06, 0x00, 0x04, 0xd8, 0x3a, /* .......: */
        0xd2, 0xe4                                      /* .. */
    };

    static const unsigned char rr_ptr[] = {
        /*-snip--*/ 0x44, 0xa2, 0x81, 0x80, 0x00, 0x01, /* .wD..... */
        0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0x32, /* .......2 */
        0x32, 0x38, 0x03, 0x32, 0x31, 0x30, 0x02, 0x35, /* 28.210.5 */
        0x38, 0x03, 0x32, 0x31, 0x36, 0x07, 0x69, 0x6e, /* 8.216.in */
        0x2d, 0x61, 0x64, 0x64, 0x72, 0x04, 0x61, 0x72, /* -addr.ar */
        0x70, 0x61, 0x00, 0x00, 0x0c, 0x00, 0x01, 0xc0, /* pa...... */
        0x0c, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x29, /* .......) */
        0xc1, 0x00, 0x1c, 0x10, 0x6d, 0x72, 0x73, 0x30, /* ....mrs0 */
        0x34, 0x73, 0x31, 0x30, 0x2d, 0x69, 0x6e, 0x2d, /* 4s10-in- */
        0x66, 0x32, 0x32, 0x38, 0x05, 0x31, 0x65, 0x31, /* f228.1e1 */
        0x30, 0x30, 0x03, 0x6e, 0x65, 0x74, 0x00, 0xc0, /* 00.net.. */
        0x0c, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x29, /* .......) */
        0xc1, 0x00, 0x11, 0x0e, 0x6d, 0x72, 0x73, 0x30, /* ....mrs0 */
        0x34, 0x73, 0x31, 0x30, 0x2d, 0x69, 0x6e, 0x2d, /* 4s10-in- */
        0x66, 0x34, 0xc0, 0x4a                          /* f4.J */
    };

    static const unsigned char rr_aaaa[] = {
        /*-snip--*/ 0x03, 0xbf, 0x81, 0x80, 0x00, 0x01, /* &L...... */
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x77, /* .......w */
        0x77, 0x77, 0x06, 0x67, 0x6f, 0x6f, 0x67, 0x6c, /* ww.googl */
        0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x1c, /* e.com... */
        0x00, 0x01, 0xc0, 0x0c, 0x00, 0x1c, 0x00, 0x01, /* ........ */
        0x00, 0x00, 0x00, 0xf0, 0x00, 0x10, 0x2a, 0x00, /* ......*. */
        0x14, 0x50, 0x40, 0x06, 0x08, 0x03, 0x00, 0x00, /* .P@..... */
        0x00, 0x00, 0x00, 0x00, 0x20, 0x04              /* .... . */
    };

    // clang-format on

    dns::Answer answer;
    ErrorOr<const unsigned char *> mebbe;

    SECTION("In case of initial ares_expand_name() failure") {
        mebbe = dns::parse_rr_impl<ares_expand_name_fail>(
            rr_a + 32, rr_a, sizeof(rr_a) / sizeof(rr_a[0]), answer,
            Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code ==
                dns::MalformedEncodedDomainNameError().code);
    }

    SECTION("In case of short packet") {
        mebbe = dns::parse_rr_impl<ares_expand_name>(
            rr_a + 32, rr_a, sizeof(rr_a) / sizeof(rr_a[0]) - 14, answer,
            Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code ==
                dns::NoSpaceForResourceRecordHeaderError().code);
    }

    SECTION("In case of excessive length") {

        // clang-format off
        static const unsigned char rr_a_excessive[] = {
            /*-snip--*/ 0x0c, 0x02, 0x81, 0x80, 0x00, 0x01, /* ........ */
            0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x77, /* .......w */
            0x77, 0x77, 0x06, 0x67, 0x6f, 0x6f, 0x67, 0x6c, /* ww.googl */
            0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, /* e.com... */
            0x00, 0x01, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, /* ........ */
            0x00, 0x00, 0x01, 0x06, 0xff, 0x04, 0xd8, 0x3a, /* .......: */
            0xd2, 0xe4                                      /* .. */
        };
        // clang-format on

        mebbe = dns::parse_rr_impl<ares_expand_name>(
            rr_a_excessive + 32, rr_a_excessive,
            sizeof(rr_a_excessive) / sizeof(rr_a_excessive[0]), answer,
            Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code ==
                dns::NoSpaceForResourceRecordError().code);
    }

    SECTION("When type is A and DLEN != 4") {

        // clang-format off
        static const unsigned char rr_a_five[] = {
            /*-snip--*/ 0x0c, 0x02, 0x81, 0x80, 0x00, 0x01, /* ........ */
            0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x77, /* .......w */
            0x77, 0x77, 0x06, 0x67, 0x6f, 0x6f, 0x67, 0x6c, /* ww.googl */
            0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, /* e.com... */
            0x00, 0x01, 0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, /* ........ */
            0x00, 0x00, 0x01, 0x06, 0x00, 0x03, 0xd8, 0x3a, /* .......: */
            0xd2, 0xe4                                      /* .. */
        };
        // clang-format on

        mebbe = dns::parse_rr_impl<ares_expand_name>(
            rr_a_five + 32, rr_a_five, sizeof(rr_a_five) / sizeof(rr_a_five[0]),
            answer, Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code == dns::InvalidRecordLengthError().code);
    }

    SECTION("In case of inet_ntop_failure") {
        mebbe = dns::parse_rr_impl<ares_expand_name, ares_expand_name,
                                   inet_ntop_fail>(
            rr_a + 32, rr_a, sizeof(rr_a) / sizeof(rr_a[0]), answer,
            Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code == dns::InetNtopError().code);
    }

    SECTION("On valid RR of type A") {
        mebbe = dns::parse_rr(rr_a + 32, rr_a, sizeof(rr_a) / sizeof(rr_a[0]),
                              answer, Logger::global());
        REQUIRE(!!mebbe);
        REQUIRE(*mebbe == rr_a + sizeof(rr_a) / sizeof(rr_a[0]));
        REQUIRE(answer.name == "www.google.com");
        REQUIRE(answer.type == dns::MK_DNS_TYPE_A);
        REQUIRE(answer.aclass == dns::MK_DNS_CLASS_IN);
        REQUIRE(answer.ttl == 262);
        REQUIRE(answer.dlen == 4);
        REQUIRE(answer.ipv4 == "216.58.210.228");
    }

    SECTION("On error when parsing a PTR response") {
        mebbe = dns::parse_rr_impl<ares_expand_name, ares_expand_name_fail>(
            rr_ptr + 45, rr_ptr, sizeof(rr_ptr) / sizeof(rr_ptr[0]), answer,
            Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code ==
                dns::MalformedEncodedDomainNameError().code);
    }

    SECTION("On valid RR of type PTR") {
        mebbe = dns::parse_rr_impl<ares_expand_name, ares_expand_name>(
            rr_ptr + 45, rr_ptr, sizeof(rr_ptr) / sizeof(rr_ptr[0]), answer,
            Logger::global());
        REQUIRE(!!mebbe);
        REQUIRE(*mebbe == rr_ptr + 85);
        REQUIRE(answer.name == "228.210.58.216.in-addr.arpa");
        REQUIRE(answer.type == dns::MK_DNS_TYPE_PTR);
        REQUIRE(answer.aclass == dns::MK_DNS_CLASS_IN);
        REQUIRE(answer.ttl == 10689);
        REQUIRE(answer.dlen == 28);
        REQUIRE(answer.hostname == "mrs04s10-in-f228.1e100.net");
    }

    SECTION("When type is AAAA and DLEN != 16") {

        // clang-format off
        static const unsigned char rr_aaaa_five[] = {
            /*-snip--*/ 0x03, 0xbf, 0x81, 0x80, 0x00, 0x01, /* &L...... */
            0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x77, /* .......w */
            0x77, 0x77, 0x06, 0x67, 0x6f, 0x6f, 0x67, 0x6c, /* ww.googl */
            0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x1c, /* e.com... */
            0x00, 0x01, 0xc0, 0x0c, 0x00, 0x1c, 0x00, 0x01, /* ........ */
            0x00, 0x00, 0x00, 0xf0, 0x00, 0x05, 0x2a, 0x00, /* ......*. */
            0x14, 0x50, 0x40, 0x06, 0x08, 0x03, 0x00, 0x00, /* .P@..... */
            0x00, 0x00, 0x00, 0x00, 0x20, 0x04              /* .... . */
        };
        // clang-format on

        mebbe = dns::parse_rr_impl<ares_expand_name>(
            rr_aaaa_five + 32, rr_aaaa_five,
            sizeof(rr_aaaa_five) / sizeof(rr_aaaa_five[0]), answer,
            Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code == dns::InvalidRecordLengthError().code);
    }

    SECTION("In case of inet_ntop_failure") {
        mebbe = dns::parse_rr_impl<ares_expand_name, ares_expand_name,
                                   inet_ntop_fail>(
            rr_aaaa + 32, rr_aaaa, sizeof(rr_aaaa) / sizeof(rr_aaaa[0]), answer,
            Logger::global());
        REQUIRE(!mebbe);
        REQUIRE(mebbe.as_error().code == dns::InetNtopError().code);
    }

    SECTION("On valid RR of type AAAA") {
        mebbe = dns::parse_rr(rr_aaaa + 32, rr_aaaa,
                              sizeof(rr_aaaa) / sizeof(rr_aaaa[0]), answer,
                              Logger::global());
        REQUIRE(!!mebbe);
        REQUIRE(*mebbe == rr_aaaa + sizeof(rr_aaaa) / sizeof(rr_aaaa[0]));
        REQUIRE(answer.name == "www.google.com");
        REQUIRE(answer.type == dns::MK_DNS_TYPE_AAAA);
        REQUIRE(answer.aclass == dns::MK_DNS_CLASS_IN);
        REQUIRE(answer.ttl == 240);
        REQUIRE(answer.dlen == 16);
        REQUIRE(answer.ipv6 == "2a00:1450:4006:803::2004");
    }
}

static ErrorOr<const unsigned char *>
parse_header_fail(const unsigned char *, const unsigned char *, size_t,
                  Var<dns::Message>, Var<Logger>) {
    return MockedError();
}

static ErrorOr<const unsigned char *> parse_question_fail(const unsigned char *,
                                                          const unsigned char *,
                                                          size_t, dns::Query &,
                                                          Var<Logger>) {
    return MockedError();
}

static ErrorOr<const unsigned char *> parse_rr_fail(const unsigned char *,
                                                    const unsigned char *,
                                                    size_t, dns::Answer &,
                                                    Var<Logger>) {
    return MockedError();
}

TEST_CASE("parse_into_impl() works as expected") {

    // clang-format off
    static const unsigned char rr_any[] = {
        /*-snip--*/ 0xcc, 0xa9, 0x81, 0x80, 0x00, 0x01, /* ........ */
        0x00, 0x09, 0x00, 0x01, 0x00, 0x01, 0x06, 0x67, /* .......g */
        0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x03, 0x63, 0x6f, /* oogle.co */
        0x6d, 0x00, 0x00, 0xff, 0x00, 0x01, 0xc0, 0x0c, /* m....... */
        0x00, 0x0f, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2d, /* .......- */
        0x00, 0x11, 0x00, 0x28, 0x04, 0x61, 0x6c, 0x74, /* ...(.alt */
        0x33, 0x05, 0x61, 0x73, 0x70, 0x6d, 0x78, 0x01, /* 3.aspmx. */
        0x6c, 0xc0, 0x0c, 0xc0, 0x0c, 0x00, 0x0f, 0x00, /* l....... */
        0x01, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x04, 0x00, /* ....-... */
        0x0a, 0xc0, 0x2f, 0xc0, 0x0c, 0x00, 0x0f, 0x00, /* ../..... */
        0x01, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x09, 0x00, /* ....-... */
        0x32, 0x04, 0x61, 0x6c, 0x74, 0x34, 0xc0, 0x2f, /* 2.alt4./ */
        0xc0, 0x0c, 0x00, 0x0f, 0x00, 0x01, 0x00, 0x00, /* ........ */
        0x00, 0x2d, 0x00, 0x09, 0x00, 0x14, 0x04, 0x61, /* .-.....a */
        0x6c, 0x74, 0x31, 0xc0, 0x2f, 0xc0, 0x0c, 0x00, /* lt1./... */
        0x0f, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2d, 0x00, /* ......-. */
        0x09, 0x00, 0x1e, 0x04, 0x61, 0x6c, 0x74, 0x32, /* ....alt2 */
        0xc0, 0x2f, 0xc0, 0x0c, 0x00, 0x1c, 0x00, 0x01, /* ./...... */
        0x00, 0x00, 0x00, 0x54, 0x00, 0x10, 0x2a, 0x00, /* ...T..*. */
        0x14, 0x50, 0x40, 0x02, 0x08, 0x02, 0x00, 0x00, /* .P@..... */
        0x00, 0x00, 0x00, 0x00, 0x20, 0x0e, 0xc0, 0x0c, /* .... ... */
        0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x02, /* ........ */
        0x00, 0x04, 0xd8, 0x3a, 0xc6, 0x2e, 0xc0, 0x0c, /* ...:.... */
        0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x21, 0x22, /* ......!" */
        0x00, 0x06, 0x03, 0x6e, 0x73, 0x31, 0xc0, 0x0c, /* ...ns1.. */
        0xc0, 0x0c, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, /* ........ */
        0x21, 0x22, 0x00, 0x06, 0x03, 0x6e, 0x73, 0x34, /* !"...ns4 */
        0xc0, 0x0c, 0xc0, 0x0c, 0x00, 0x02, 0x00, 0x01, /* ........ */
        0x00, 0x00, 0x21, 0x22, 0x00, 0x06, 0x03, 0x6e, /* ..!"...n */
        0x73, 0x32, 0xc0, 0x0c, 0xc0, 0x0c, 0x00, 0x02, /* s2...... */
        0x00, 0x01, 0x00, 0x00, 0x21, 0x22, 0x00, 0x06, /* ....!".. */
        0x03, 0x6e, 0x73, 0x33, 0xc0, 0x0c              /* .ns3.. */
    };
    // clang-format on

    std::string pkt{(const char *)rr_any, sizeof(rr_any) / sizeof(rr_any[0])};
    Var<dns::Message> message{new dns::Message};
    Var<Logger> logger = Logger::global();
    Error err;

    SECTION("On parse_header failure") {
        err = dns::parse_into_impl<parse_header_fail>(message, pkt, logger);
        REQUIRE(err.code == MockedError().code);
    }

    SECTION("On parse_question failure") {
        err = dns::parse_into_impl<dns::parse_header, parse_question_fail>(
            message, pkt, logger);
        REQUIRE(err.code == MockedError().code);
    }

    SECTION("On parse_rr failure for answers") {
        err = dns::parse_into_impl<dns::parse_header, dns::parse_question,
                                   parse_rr_fail>(message, pkt, logger);
        REQUIRE(err.code == MockedError().code);
    }

    SECTION("On parse_rr failure for authority") {
        err = dns::parse_into_impl<dns::parse_header, dns::parse_question,
                                   dns::parse_rr, parse_rr_fail>(message, pkt,
                                                                 logger);
        REQUIRE(err.code == MockedError().code);
    }

    SECTION("On parse_rr failure for additional") {
        err = dns::parse_into_impl<dns::parse_header, dns::parse_question,
                                   dns::parse_rr, dns::parse_rr, parse_rr_fail>(
            message, pkt, logger);
        REQUIRE(err.code == MockedError().code);
    }
}
