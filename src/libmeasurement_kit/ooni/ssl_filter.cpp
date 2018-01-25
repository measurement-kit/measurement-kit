// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ooni/ssl_filter.hpp"

#include <stdexcept>
#include <iostream>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <sys/time.h>

namespace mk {
namespace ooni {

SSLFilter::SSLFilter(SSL_CTX* ctxt,
                     SharedPtr<std::string> nread,
                     SharedPtr<std::string> nwrite,
                     SharedPtr<std::string> aread,
                     SharedPtr<std::string> awrite,
                     std::string hostname)
                      :
                     nread(nread),
                     nwrite(nwrite),
                     aread(aread),
                     awrite(awrite) {

    rbio = BIO_new(BIO_s_mem());
    wbio = BIO_new(BIO_s_mem());

    ssl = SSL_new(ctxt);
    if (ssl == NULL) {
        exit(-1); //XXX
    }

    //XXX for now, we're only doing client-side,
	// but maybe make it configurable?
    //SSL_set_accept_state(ssl);
    SSL_set_connect_state(ssl);
    SSL_set_bio(ssl, rbio, wbio);
    int success = SSL_set_tlsext_host_name(ssl, hostname.c_str()); //XXX error code
}

SSLFilter::~SSLFilter() {
    SSL_free(ssl);
}

// XXX ought we to specify FilterDirection?
//void SSLFilter::update(Filter::FilterDirection) {
void SSLFilter::update() {
    // If we have data from the network to process, put it the memory BIO for OpenSSL
    if (!nread->empty()) {
        int written = BIO_write(rbio, nread->c_str(), nread->length());
        if (written > 0) {
            nread->erase(0, written);
        }
    }

    // If the application wants to write data out to the network, process it with SSL_write
    if (!awrite->empty()) {
        int written = SSL_write(ssl, awrite->c_str(), awrite->length());

        if (!continue_ssl_(written)) {
            throw std::runtime_error("An SSL error occured.");
        }

        if (written > 0) {
            awrite->erase(0, written);
        }
    }

    // Read data for the application from the encrypted connection and place it in the string for the app to read
    while (1) {
        char *readto = new char[1024];
        int read = SSL_read(ssl, readto, 1024);

        if (!continue_ssl_(read)) {
            delete readto;
            throw std::runtime_error("An SSL error occured.");
        }

        if (read > 0) {
            size_t cur_size = aread->length();
            aread->resize(cur_size + read);
            std::copy(readto, readto + read, aread->begin() + cur_size);
        }

        delete readto;

        if (static_cast<size_t>(read) != 1024 || read == 0) break;
    }

    // Read any data to be written to the network from the memory BIO and copy it to nwrite
    while (1) {
        char *readto = new char[1024];
        int read = BIO_read(wbio, readto, 1024);

        if (read > 0) {
            size_t cur_size = nwrite->length();
            nwrite->resize(cur_size + read);
            std::copy(readto, readto + read, nwrite->begin() + cur_size);
        }

        delete readto;

        if (static_cast<size_t>(read) != 1024 || read == 0) break;
    }
}
bool SSLFilter::continue_ssl_(int function_return) {
    int err = SSL_get_error(ssl, function_return);
    char desc[120];

    if (err != SSL_ERROR_NONE) {
        ERR_error_string(err, desc);
        ERR_print_errors_fp(stdout);
    }

    if (err == SSL_ERROR_NONE || err == SSL_ERROR_WANT_READ) {
        return true;
    }

    if (err == SSL_ERROR_SYSCALL) {
        ERR_print_errors_fp(stderr);
        perror("syscall error: ");
        return false;
    }

    if (err == SSL_ERROR_SSL) {
        ERR_print_errors_fp(stderr);
        return false;
    }
    return true;
}

}
}
