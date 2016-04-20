// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <openssl/ssl.h>
#include <measurement_kit/net/error.hpp>
#include <strings.h>

#include "src/net/ssl-validate.hpp"

namespace mk {
namespace net {

typedef enum {
    MatchFound,
    MatchNotFound,
    NoSANPresent,
    MalformedCertificate,
    ValidationError
} HostnameValidationResult;

// This function is taken from:
// https://github.com/libressl-portable/openbsd/blob/eea365c0133c49cf771f299bdcfe1dcfc7950556/src/lib/libtls/tls_verify.c
static int match_name(const char *cert_name, const char *name) {
    const char *cert_domain, *domain, *next_dot;

    if (strcasecmp(cert_name, name) == 0)
        return 0;

    /* Wildcard match? */
    if (cert_name[0] == '*') {
        /*
         * Valid wildcards:
         * - "*.domain.tld"
         * - "*.sub.domain.tld"
         * - etc.
         * Reject "*.tld".
         * No attempt to prevent the use of eg. "*.co.uk".
         */
        cert_domain = &cert_name[1];
        /* Disallow "*"  */
        if (cert_domain[0] == '\0')
            return -1;
        /* Disallow "*foo" */
        if (cert_domain[0] != '.')
            return -1;
        /* Disallow "*.." */
        if (cert_domain[1] == '.')
            return -1;
        next_dot = strchr(&cert_domain[1], '.');
        /* Disallow "*.bar" */
        if (next_dot == NULL)
            return -1;
        /* Disallow "*.bar.." */
        if (next_dot[1] == '.')
            return -1;

        domain = strchr(name, '.');

        /* No wildcard match against a name with no host part. */
        if (name[0] == '.')
            return -1;
        /* No wildcard match against a name with no domain part. */
        if (domain == NULL || strlen(domain) == 1)
            return -1;

        if (strcasecmp(cert_domain, domain) == 0)
            return 0;
    }

    return -1;

}

// The following two functions are borrowed from:
// https://github.com/iSECPartners/ssl-conservatory/blob/5465a20b2293e498d172ed8278478195b441b8d4/openssl/openssl_hostname_validation.c
static HostnameValidationResult matches_subject_alternative_name(const char *name,
                                                                 const X509 *server_cert) {
    HostnameValidationResult result = MatchNotFound;
    int i;
    int san_names_nb = -1;
    STACK_OF(GENERAL_NAME) *san_names = NULL;

    // Try to extract the names within the SAN extension from the certificate
    san_names = (STACK_OF(GENERAL_NAME) *) X509_get_ext_d2i((X509 *)server_cert, NID_subject_alt_name, NULL, NULL);
    if (san_names == NULL) {
        return NoSANPresent;
    }
    san_names_nb = sk_GENERAL_NAME_num(san_names);

    // Check each name within the extension
    for (i = 0; i < san_names_nb; i++) {
        const GENERAL_NAME *current_name = sk_GENERAL_NAME_value(san_names, i);

        if (current_name->type == GEN_DNS) {
            // Current name is a DNS name, let's check it
            char *dns_name = (char *)ASN1_STRING_data(current_name->d.dNSName);

            // Make sure there isn't an embedded NUL character in the DNS name
            if (ASN1_STRING_length(current_name->d.dNSName) != strlen(dns_name)) {
                result = MalformedCertificate;
                break;
            } else { // Compare expected name with the DNS name
                if (match_name(dns_name, name) == 0) {
                    result = MatchFound;
                    break;
                }
            }
        }
    }
    sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);

    return result;
}

static HostnameValidationResult matches_common_name(const char *name, const X509 *server_cert) {
    int common_name_loc = -1;
    X509_NAME_ENTRY *common_name_entry = NULL;
    ASN1_STRING *common_name_asn1 = NULL;
    char *common_name_str = NULL;

    // Find the position of the CN field in the Subject field of the certificate
    common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name((X509 *) server_cert), NID_commonName, -1);
    if (common_name_loc < 0) {
        return ValidationError;
    }

    // Extract the CN field
    common_name_entry = X509_NAME_get_entry(X509_get_subject_name((X509 *) server_cert), common_name_loc);
    if (common_name_entry == NULL) {
        return ValidationError;
    }

    // Convert the CN field to a C string
    common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
    if (common_name_asn1 == NULL) {
        return ValidationError;
    }
    common_name_str = (char *) ASN1_STRING_data(common_name_asn1);

    // Make sure there isn't an embedded NUL character in the CN
    if (ASN1_STRING_length(common_name_asn1) != strlen(common_name_str)) {
        return MalformedCertificate;
    }

    // Compare expected name with the CN
    if (match_name(common_name_str, name) == 0) {
        return MatchFound;
    }
    else {
        return MatchNotFound;
    }
}

Error ssl_validate_hostname(std::string hostname, const X509 *server_cert) {
    HostnameValidationResult result;

    result = matches_subject_alternative_name(hostname.c_str(), server_cert);
    if (result == NoSANPresent) {
        result = matches_common_name(hostname.c_str(), server_cert);
    }

    if (result == MatchFound) {
         return NoError();
    }
    return SSLInvalidHostnameError();
}

}
}
