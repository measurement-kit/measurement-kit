// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_SSL_FILTER_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_SSL_FILTER_HPP

#include "src/libmeasurement_kit/ooni/utils.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

namespace mk {
namespace ooni {

class SSLFilter {
    public:
        SSLFilter(SSL_CTX* ctxt,
                  SharedPtr<std::string> nread,
                  SharedPtr<std::string> nwrite,
                  SharedPtr<std::string> aread,
                  SharedPtr<std::string> awrite,
                  std::string hostname);
        virtual ~SSLFilter();

        void update();

    private:
        bool continue_ssl_(int function_return);

        SSL * ssl;
        BIO * rbio;
        BIO * wbio;

        SharedPtr<std::string> nread;
        SharedPtr<std::string> nwrite;
        SharedPtr<std::string> aread;
        SharedPtr<std::string> awrite;
};

}
}
#endif
