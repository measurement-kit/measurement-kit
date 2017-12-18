// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/ooni/constants.hpp"
#include "private/ooni/utils.hpp"
#include "private/common/fcompose.hpp"
#include "private/common/utils.hpp"
#include <measurement_kit/ooni.hpp>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <sys/time.h>




namespace mk {
namespace ooni {

using namespace mk::report;



class SSLFilter {
    public:
        SSLFilter(SSL_CTX* ctxt,
                  std::shared_ptr<std::string> nread,
                  std::shared_ptr<std::string> nwrite,
                  std::shared_ptr<std::string> aread,
                  std::shared_ptr<std::string> awrite,
                  std::string hostname);
        virtual ~SSLFilter();

        void update();

    private:
        bool continue_ssl_(int function_return);

        SSL * ssl;
        BIO * rbio;
        BIO * wbio;

        std::shared_ptr<std::string> nread;
        std::shared_ptr<std::string> nwrite;
        std::shared_ptr<std::string> aread;
        std::shared_ptr<std::string> awrite;
};

SSLFilter::SSLFilter(SSL_CTX* ctxt,
                     std::shared_ptr<std::string> nread,
                     std::shared_ptr<std::string> nwrite,
                     std::shared_ptr<std::string> aread,
                     std::shared_ptr<std::string> awrite,
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

std::string fragmented_https_request(bool do_fragment, bool do_ssl,
                        SharedPtr<Logger> logger) {
    std::string hostname { "example.com" };
    std::string ip { "93.184.216.34" };

    auto nread = std::make_shared<std::string>();
    auto nwrite = std::make_shared<std::string>();
    auto aread = std::make_shared<std::string>();
    auto awrite = std::make_shared<std::string>();

    SSL_CTX* ctx = SSL_CTX_new(SSLv23_method()); //XXX don't ignore errors
    if (ctx == NULL) {
        logger->info("failed to make context\n");
        exit(1);
    }   
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL); //XXX probably want to verify eventually

    SSLFilter ssl_filter = SSLFilter(ctx, nread, nwrite, aread, awrite, hostname);
    if (!do_ssl) { //XXX v. hacky; make passthrough class
        awrite = nwrite;
        aread = nread;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        logger->info("socket() failed");
        return *aread; //XXX probably raise exception
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    if (do_ssl) {
        serv_addr.sin_port = htons(443);
    } else {
        serv_addr.sin_port = htons(80);
    }

    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        logger->info("inet_pton() failed");
        return *aread; //XXX probably raise exception
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        logger->info("connect() failed");
        return *aread; //XXX probably raise exception
    }

    *awrite = "GET /index.html HTTP/1.1\r\nHost: ";
    *awrite += hostname;
    *awrite += "\r\n\r\n";
    if (do_ssl) { //XXX hacky; make a proper passthrough class
        ssl_filter.update();
    }

    char readbuf[1024]; //XXX put on heap

    struct timeval tv;
    int sel_ret;
    int n;
    bool stop_reading = false;
    bool stop_writing = false;
    fd_set rfds, wfds;

    while(1) {
        logger->info("TOP OF WHILE LOOP\n");
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        if (!stop_reading) {
            FD_SET(sockfd, &rfds);
            logger->info("we want to read");
        }
        if (!nwrite->empty() && !stop_writing) {
            FD_SET(sockfd, &wfds);
            logger->info("we want to write");
        }

        tv.tv_sec = 4;
        tv.tv_usec = 0;

        logger->info("BLOCKING ON SELECT\n");
        sel_ret = select(sockfd+1, &rfds, &wfds, NULL, &tv);
        if (sel_ret < 0) {
            logger->info("select() failed");
            break;
        }
        if (sel_ret == 0) {
            logger->info("select() timed out.\n");
            if (do_ssl) {
                ssl_filter.update(); //XXX shouldn't need to sprinkle these around so much if i do more thinking
            }
            break;
        }
        if (FD_ISSET(sockfd, &wfds)) {
            logger->info("WRITABLE\n");

            //XXX clean this up
            if (do_fragment) {
                std::size_t found = nwrite->find(hostname);
                if (found!=std::string::npos) {
                    logger->info("$$$$$$ found plaintext hostname; hacking planet $$$$$$$$$");
                    n = write(sockfd, nwrite->c_str(), found+3);
                    if (n > 0) {
                        nwrite->erase(0, n);
                    }
                    sleep(0.5);
                }
            }

            n = write(sockfd, nwrite->c_str(), nwrite->length());
            if (n > 0) {
                nwrite->erase(0, n);
            }
        }

        if (FD_ISSET(sockfd, &rfds)) {
            logger->info("READABLE\n");
            while(1) {
                n = read(sockfd, &readbuf, sizeof(readbuf));
                logger->info("read from sockfd: %d", n);
                if (read > 0) {
                    size_t cur_size = nread->length();
                    nread->resize(cur_size + n);
                    std::copy(readbuf, readbuf + n, nread->begin() + cur_size);
                }
                if (n == 0) {
                    stop_reading = true;
                    break;
                }
                if (static_cast<size_t>(n) != sizeof(readbuf)) {
                    break;
                }
                if(n < 0) {
                    break;
                }
            }
        }

        if (do_ssl) {
            ssl_filter.update();
        }
        if (awrite->size() == 0 && nwrite->size() == 0) {
            stop_writing = true;
        }
        if (stop_reading && stop_writing) {
            break;
        }

    }

    return *aread;
}




void dpi_fragment(Settings options, Callback<SharedPtr<report::Entry>> callback,
                        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    reactor->call_in_thread(logger, [=]() {
        logger->info("starting dpi_fragment");
        SharedPtr<Entry> entry(new Entry);

        std::string fragmented_https = fragmented_https_request(true, true, logger);
        std::string unfragmented_https = fragmented_https_request(false, true, logger);
        std::string fragmented_http = fragmented_https_request(true, false, logger);
        std::string unfragmented_http = fragmented_https_request(false, false, logger);

        logger->info("fragmented https response: %s", fragmented_https.c_str());
        logger->info("unfragmented https response: %s", unfragmented_https.c_str());
        logger->info("fragmented http response: %s", fragmented_http.c_str());
        logger->info("unfragmented http response: %s", unfragmented_http.c_str());

        logger->info("fragmented https response length: %d", fragmented_https.length());
        logger->info("unfragmented https response length: %d", unfragmented_https.length());
        logger->info("fragmented http response length: %d", fragmented_http.length());
        logger->info("unfragmented http response length: %d", unfragmented_http.length());
        callback(entry);
    });
}

} // namespace ooni
} // namespace mk
