// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_DNS_ERROR_HPP
#define MEASUREMENT_KIT_DNS_ERROR_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace dns {

MK_DEFINE_ERR(MK_ERR_DNS(0), FormatError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(1), ServerFailedError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(2), NotExistError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(3), NotImplementedError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(4), RefusedError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(5), TruncatedError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(6), UnknownError, "dns_unknown_error")
using TimeoutError = mk::TimeoutError; /* Was: MK_ERR_DNS(7) */
MK_DEFINE_ERR(MK_ERR_DNS(8), ShutdownError, "dns_shutdown")
MK_DEFINE_ERR(MK_ERR_DNS(9), CancelError, "dns_cancel")
MK_DEFINE_ERR(MK_ERR_DNS(10), NoDataError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(11), InvalidIPv4AddressError, "dns_invalid_ipv4")
MK_DEFINE_ERR(MK_ERR_DNS(12), InvalidIPv6AddressError, "dns_invalid_ipv6")
MK_DEFINE_ERR(MK_ERR_DNS(13), UnsupportedClassError, "dns_unsupported_class")
MK_DEFINE_ERR(MK_ERR_DNS(14), InvalidNameForPTRError, "dns_invalid_ptr")
MK_DEFINE_ERR(MK_ERR_DNS(15), ResolverError, "dns_resolver_error")
MK_DEFINE_ERR(MK_ERR_DNS(16), UnsupportedTypeError, "dns_unsupported_type")
MK_DEFINE_ERR(MK_ERR_DNS(17), InvalidDnsEngine, "dns_invalid_engine")

// getaddrinfo errors
MK_DEFINE_ERR(MK_ERR_DNS(18), TemporaryFailureError, "dns_temporary_failure")
MK_DEFINE_ERR(MK_ERR_DNS(19), InvalidFlagsValueError, "dns_invalid_flags")
MK_DEFINE_ERR(MK_ERR_DNS(20), InvalidHintsValueError, "dns_invalid_hints")
MK_DEFINE_ERR(MK_ERR_DNS(21), NonRecoverableFailureError,
              "dns_non_recoverable_failure")
MK_DEFINE_ERR(MK_ERR_DNS(22), NotSupportedAIFamilyError,
              "dns_unsupported_family")
MK_DEFINE_ERR(MK_ERR_DNS(23), MemoryAllocationFailureError, "dns_memory_error")
MK_DEFINE_ERR(MK_ERR_DNS(24), HostOrServiceNotProvidedOrNotKnownError,
              "dns_host_or_service_not_provided_or_not_known")
MK_DEFINE_ERR(MK_ERR_DNS(25), ArgumentBufferOverflowError, "dns_overflow_error")
MK_DEFINE_ERR(MK_ERR_DNS(26), UnknownResolvedProtocolError,
              "dns_unknown_protocol")
MK_DEFINE_ERR(MK_ERR_DNS(27), NotSupportedServnameError,
              "dns_unsupported_servname")
MK_DEFINE_ERR(MK_ERR_DNS(28), NotSupportedAISocktypeError,
              "dns_unsupported_socktype")
//Was: MK_DEFINE_ERR(MK_ERR_DNS(29), InetNtopFailureError,
//                   "dns_inet_ntop_failure")

} // namespace dns
} // namespace mk
#endif
