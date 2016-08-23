// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/error.hpp>
#include <measurement_kit/dns.hpp>

#include <event2/dns.h>

namespace mk {
namespace dns {

Error dns_error(int code) {
    Error err;

    //
    // Here we map evdns error codes to specific Errors.
    //
    // We start with errors specified in RFC 1035 (see also event2/dns.h).
    //

    if (code == DNS_ERR_NONE) {
        err = NoError();

    } else if (code == DNS_ERR_FORMAT) {
        // The name server was unable to interpret the query
        err = FormatError();

    } else if (code == DNS_ERR_SERVERFAILED) {
        // The name server was unable to process this query due to a
        // problem with the name server
        err = ServerFailedError();

    } else if (code == DNS_ERR_NOTEXIST) {
        // The domain name does not exist
        err = NotExistError();

    } else if (code == DNS_ERR_NOTIMPL) {
        // The name server does not support the requested kind of query
        err = NotImplementedError();

    } else if (code == DNS_ERR_REFUSED) {
        // The name server refuses to perform the specified operation
        // for policy reasons
        err = RefusedError();

        //
        // The following are evdns specific errors
        //

    } else if (code == DNS_ERR_TRUNCATED) {
        // The reply was truncated or ill-formatted
        err = TruncatedError();

    } else if (code == DNS_ERR_UNKNOWN) {
        // An unknown error occurred
        err = UnknownError();

    } else if (code == DNS_ERR_TIMEOUT) {
        // Communication with the server timed out
        err = TimeoutError();

    } else if (code == DNS_ERR_SHUTDOWN) {
        // The request was canceled because the DNS subsystem was shut down.
        err = ShutdownError();

    } else if (code == DNS_ERR_CANCEL) {
        // The request was canceled via a call to evdns_cancel_request
        err = CancelError();

    } else if (code == DNS_ERR_NODATA) {
        // There were no answers and no error condition in the DNS packet.
        // This can happen when you ask for an address that exists, but
        // a record type that doesn't.
        err = NoDataError();

    } else {
        // Safery net - should really not happen
        err = GenericError();
    }

    return err;
}

} // namespace dns
} // namespace mk
