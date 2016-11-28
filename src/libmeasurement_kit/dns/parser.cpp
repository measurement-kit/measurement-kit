// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../dns/parser_impl.hpp"

namespace mk {
namespace dns {

ErrorOr<const unsigned char *>
parse_question(const unsigned char *aptr, const unsigned char *abuf,
               size_t alen, Query &query, Var<Logger> logger) {
    return parse_question_impl(aptr, abuf, alen, query, logger);
}

ErrorOr<const unsigned char *> parse_rr(
        const unsigned char *aptr, const unsigned char *abuf, size_t alen,
        Answer &answer, Var<Logger> logger) {
    return parse_rr_impl(aptr,abuf, alen, answer, logger);
}

Error parse_into(Var<Message> message, std::string packet, Var<Logger> logger) {
    return parse_into_impl(message, packet, logger);
}

} // namespace dns
} // namespace mk
