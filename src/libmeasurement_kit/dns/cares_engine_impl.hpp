// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_CARES_ENGINE_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_CARES_ENGINE_IMPL_HPP

#include <event2/util.h>

#include "../dns/cares_engine.hpp"
#include "../dns/parser.hpp"
#include "../dns/sendrecv.hpp"
#include "../dns/serializer.hpp"

namespace mk {
namespace dns {

template <MK_MOCK(serialize), MK_MOCK(sendrecv), MK_MOCK(parse_into),
          MK_MOCK(evutil_secure_rng_get_bytes)>
void cares_engine_query_impl(QueryClass dns_class, QueryType dns_type,
                             std::string name, Callback<Error, Var<Message>> cb,
                             Settings settings, Var<Reactor> reactor,
                             Var<Logger> logger, int attempt) {

    /*
     * Note: `cares_engine_query()` should defer calling us using the
     * reactor->call_soon() method, so we can directly call the callback
     * here without being annoying with the caller.
     */

    // TODO: perhaps move options value checks in the common code?

    ErrorOr<int> maybe_attempts = settings.get_noexcept<int>("dns/attempts", 1);
    if (!maybe_attempts or *maybe_attempts < 1) {
        cb(InvalidAttemptsOptionError(), nullptr);
        return;
    }

    ErrorOr<uint16_t> maybe_qid =
        settings.get_noexcept<uint16_t>("dns/query_id", 0);
    if (!maybe_qid) {
        cb(InvalidQueryIdOptionError(), nullptr);
        return;
    }
    if (*maybe_qid == 0) {
        // TODO: hide this explicit dependency on libevent
        uint16_t qid;
        evutil_secure_rng_get_bytes(&qid, sizeof(qid));
        *maybe_qid = qid;
        /*
         * Note: we don't store into settings the random query-id such that
         * we can say whether a packet is a response to latest query (in case
         * the user requested us to retry and we timeout).
         */
        // FALLTHROUGH
    }

    ErrorOr<bool> maybe_rd =
        settings.get_noexcept<bool>("dns/recursion_desired", true);
    if (!maybe_rd) {
        cb(InvalidRecursionDesiredOptionError(), nullptr);
        return;
    }

    ErrorOr<bool> maybe_rc =
        settings.get_noexcept<bool>("dns/randomize_case", false);
    if (!maybe_rc) {
        cb(InvalidRandomizeCaseOptionError(), nullptr);
        return;
    }
    if (!!*maybe_rc) {
        // TODO: implement this borrowing code from libevent
        cb(NotImplementedError(), nullptr);
        return;
    }

    ErrorOr<std::vector<uint8_t>> maybe_packet =
        serialize(name, dns_class, dns_type, *maybe_qid, *maybe_rd, logger);
    if (!maybe_packet) {
        cb(maybe_packet.as_error(), nullptr);
        return;
    }

    sendrecv(
        settings.get("dns/nameserver", std::string{"8.8.8.8"}),
        settings.get("dns/port", std::string{"53"}), *maybe_packet,
        [=](Error error, std::vector<uint8_t> packet) {
            if (error) {
                if (error == TimeoutError() and attempt < *maybe_attempts) {
                    /*
                     * Note: no need to defer execution here because
                     * sendrecv() will do that for us. We call ourself
                     * again, even though this involves some extra
                     * processing, because this should not be critical
                     * to performance (refactor otherwise).
                     */
                    cares_engine_query_impl<serialize, sendrecv, parse_into>(
                        dns_class, dns_type, name, cb, settings, reactor,
                        logger, attempt + 1);
                    return;
                }
                cb(error, nullptr);
                return;
            }
            Var<Message> msg{new Message};
            error = parse_into(msg, packet, logger);
            if (error) {
                cb(error, nullptr);
                return;
            }
            if (msg->qid != *maybe_qid) {
                /*
                 * TODO: do we want to be able to tell the user whether the
                 * received query-id was part of a past query?
                 */
                cb(UnexpectedQueryIdError(), msg);
                return;
            }
            // Map msg->rcode to an existing DNS error
            Error err = GenericError();
            switch (msg->rcode) {
            case 0:
                err = NoError();
                break;
            case 1:
                err = FormatError();
                break;
            case 2:
                err = ServerFailedError();
                break;
            case 3:
                err = NotExistError();
                break;
            case 4:
                err = NotImplementedError();
                break;
            case 5:
                err = RefusedError();
                break;
            default:
                err = UnknownError();
                break;
            }
            /*
             * TODO: for OONI's sake here we may want to call `recv()` and
             * see whether we recv another response after the first one.
             */
            cb(err, msg);
        },
        settings, reactor, logger);
}

} // namespace dns
} // namespace mk
#endif
