// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/mock.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <event2/util.h>
#include <iomanip>
#include <list>
#include <measurement_kit/common.hpp>
#include <openssl/sha.h>
#include <random>
#include <regex>

struct timeval;

namespace mk {

double time_now() {
    timeval tv;
    timeval_now(&tv);
    double result = tv.tv_sec + tv.tv_usec / (double)1000000.0;
    return result;
}

Error parse_iso8601_utc(std::string ts, std::tm *tmb) {
    *tmb = {}; // "portable programs should initialize the structure"
    std::istringstream ss(ts);
    ss >> std::get_time(tmb, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
        return ValueError();
    }
    return NoError();
}

timeval *timeval_init(timeval *tv, double delta) {
    if (delta < 0) {
        return nullptr;
    }
    // Note: on Windows `struct timeval` fields are `long`
#ifndef _WIN32
    tv->tv_sec = (time_t)floor(delta);
    tv->tv_usec = (suseconds_t)((delta - floor(delta)) * 1000000);
#else
    tv->tv_sec = (long)floor(delta);
    tv->tv_usec = (long)((delta - floor(delta)) * 1000000);
#endif
    return tv;
}

std::string random_within_charset(const std::string charset, size_t length) {
    // See <http://stackoverflow.com/questions/440133/>
    if (charset.size() < 1) {
        throw std::runtime_error("random_within_charset_with_empty_charset");
    }
    auto randchar = [&charset]() {
        int rand = 0;
        evutil_secure_rng_get_bytes(&rand, sizeof (rand));
        return charset[rand % charset.size()];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

std::string random_printable(size_t length) {
    static const std::string ascii =
            " !\"#$%&\'()*+,-./"         // before numbers
            "0123456789"                 // numbers
            ":;<=>?@"                    // after numbers
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ" // uppercase
            "[\\]^_`"                    // between upper and lower
            "abcdefghijklmnopqrstuvwxyz" // lowercase
            "{|}~"                       // final
        ;
    return random_within_charset(ascii, length);
}

std::string random_str(size_t length) {
    static const std::string alnum =
            "0123456789"                 // numbers
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ" // uppercase
            "abcdefghijklmnopqrstuvwxyz" // lowercase
        ;
    return random_within_charset(alnum, length);
}

std::string random_str_uppercase(size_t length) {
    static const std::string num_upper =
            "0123456789"                  // numbers
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"  // uppercase
        ;
    return random_within_charset(num_upper, length);
}

std::string random_str_lower_alpha(size_t length) {
    static const std::string lower =
            "abcdefghijklmnopqrstuvwxyz"  // lowercase
        ;
    return random_within_charset(lower, length);
}

std::string random_tld() {
    static const std::vector<std::string> tlds
        { ".com", ".net", ".org", ".info", ".test", ".invalid" };
    return random_choice(tlds);
}

void dump_settings(Settings &s, std::string prefix, SharedPtr<Logger> logger) {
    logger->debug2("%s: {", prefix.c_str());
    for (auto pair : s) {
        logger->debug2("%s:   \"%s\": \"%s\",", prefix.c_str(),
                       pair.first.c_str(), pair.second.c_str());
    }
    logger->debug2("%s: }", prefix.c_str());
}

std::string sha256_of(std::string input) {
    // See: <http://stackoverflow.com/questions/2262386/>
    unsigned char hash[SHA256_DIGEST_LENGTH];
    constexpr size_t hash_size = sizeof(hash) / sizeof(hash[0]);
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, input.data(), input.size());
    SHA256_Final(hash, &ctx);
    std::stringstream ss;
    for (size_t i = 0; i < hash_size; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0')
           << (unsigned)hash[i];
    }
    return ss.str();
}

ErrorOr<std::string> slurp(std::string path) {
    ErrorOr<std::vector<char>> v = slurpv<char>(path);
    if (!v) {
        return {v.as_error(), std::string{}};
    }
    std::string s{v->begin(), v->end()};  /* Note that here we make a copy */
    return {NoError(), std::move(s)};
}

bool startswith(std::string s, std::string p) {
    return s.find(p) == 0;
}

/*-
 *     0 1 2 3 4 5 6
 * s: |f|o|o|b|a|r|
 * p:       |b|a|r|
 *           0 1 2 3
 *
 * s.size() - p.size() = 3
 */
bool endswith(std::string s, std::string p) {
    return s.size() >= p.size() ? s.rfind(p) == (s.size() - p.size()) : false;
}

std::string random_choice(std::vector<std::string> inputs) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(inputs.begin(), inputs.end(), g);
    return inputs[0];
}

std::string randomly_capitalize(std::string input) {
    std::random_device rd;
    std::mt19937 g(rd());
    for (auto &c: input) {
        if (g() % 2 == 0) {
            c = toupper(c);
        } else {
            c = tolower(c);
        }
    }
    return input;
}

double percentile(std::vector<double> v, double percent) {
    // Adapted from <http://code.activestate.com/recipes/511478/>
    if (v.size() <= 0) {
        throw std::runtime_error("zero length vector");
    }
    std::sort(v.begin(), v.end());
    auto pivot = (v.size() - 1) * percent;
    auto pivot_floor = floor(pivot);
    auto pivot_ceil = ceil(pivot);
    if (pivot_floor == pivot_ceil) {
        return v[int(pivot)];
    }
    auto val0 = v[int(pivot_floor)] * (pivot_ceil - pivot);
    auto val1 = v[int(pivot_ceil)] * (pivot - pivot_floor);
    return val0 + val1;
}

double median(std::vector<double> v) {
    return percentile(v, 0.5);
}

} // namespace mk
