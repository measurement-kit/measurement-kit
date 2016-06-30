// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ndt.hpp>
#include <measurement_kit/ooni.hpp>

#include <string.h>
#include <stdint.h>

using namespace mk;

#define MK_EASY_ABI 0

struct MkEasy_ {
    Var<NetTest> net_test;
};

mk_easy_abi_t mk_easy_abi_get() {
    return MK_EASY_ABI;
}

mk_easy_t *mk_easy_new(const char *test_name) {
    mk_easy_t *easy = new MkEasy_;

    if (strcmp(test_name, "ndt") == 0) {
        easy->net_test.reset(new ndt::NdtTest);
    } else if (strcmp(test_name, "ooni/dns_injection") == 0) {
        easy->net_test.reset(new ooni::DnsInjection);
    } else if (strcmp(test_name, "ooni/http_invalid_request_line") == 0) {
        easy->net_test.reset(new ooni::HttpInvalidRequestLine);
    } else if (strcmp(test_name, "ooni/tcp_connect") == 0) {
        easy->net_test.reset(new ooni::TcpConnect);
    } else {
        delete easy;
        return nullptr;
    }

    return easy;
}

void mk_easy_on_log(mk_easy_t *easy, mk_easy_log_func_t *func) {
    easy->net_test->on_log([=](uint32_t verbosity, const char *message) {
        func(verbosity, message);
    });
}

void mk_easy_set_verbosity(mk_easy_t *easy, uint32_t verbosity) {
    easy->net_test->set_verbosity(verbosity);
}

void mk_easy_increase_verbosity(mk_easy_t *easy) {
    easy->net_test->increase_verbosity();
}

void mk_easy_set_input_filepath(mk_easy_t *easy, const char *path) {
    easy->net_test->set_input_filepath(path);
}

void mk_easy_set_output_filepath(mk_easy_t *easy, const char *path) {
    easy->net_test->set_output_filepath(path);
}

void mk_easy_set_options(mk_easy_t *easy, const char *k, const char *v) {
    easy->net_test->set_options(k, v);
}

mk_easy_error_t mk_easy_run(mk_easy_t *easy) {
    easy->net_test->run();
    return 0;
}

void mk_easy_run_async(mk_easy_t *easy, mk_easy_done_func_t *func) {
    easy->net_test->run([=]() {
        func(0);
    });
}

void mk_easy_delete(mk_easy_t *easy) {
    delete easy;
}
