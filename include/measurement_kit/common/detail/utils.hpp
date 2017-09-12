// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_UTILS_HPP
#define MEASUREMENT_KIT_COMMON_UTILS_HPP

#include <measurement_kit/common/detail/mock.hpp>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <event2/util.h>
#include <iomanip>
#include <measurement_kit/common.hpp>
#include <openssl/sha.h>
#include <random>
#include <regex>

struct timeval;

namespace mk {

/*
 * There are many ways in C++ to slurp the whole content of a file into a
 * C++ string. The following is my favourite. Compared to other methods,
 * not only this seems fastest, but also it provides greater control over
 * errors that can occur. Thus mocking is simpler.
 *
 * See <http://stackoverflow.com/questions/116038>.
 */
template <typename T, MK_MOCK_AS(std::fopen, std_fopen),
          MK_MOCK_AS(std::fseek, std_fseek_1),
          MK_MOCK_AS(std::ftell, std_ftell),
          MK_MOCK_AS(std::fseek, std_fseek_2),
          MK_MOCK_AS(std::fread, std_fread),
          MK_MOCK_AS(std::fclose, std_fclose)>
ErrorOr<std::vector<T>> slurpv_impl(std::string p) {
    FILE *filep = std_fopen(p.c_str(), "rb");
    if (filep == nullptr) {
        return FileIoError();
    }
    if (std_fseek_1(filep, 0, SEEK_END) != 0) {
        std_fclose(filep);
        return FileIoError();
    }
    // Note: ftello() might be better for reading very large files but
    // honestly I do think we should use some kind of mmap for them.
    long pos = std_ftell(filep);
    if (pos < 0) {
        std_fclose(filep);
        return FileIoError();
    }
    std::vector<T> result;
    // Note: cast to unsigned safe because we excluded negative case above
    result.resize((unsigned long)pos, 0);
    if (std_fseek_2(filep, 0, SEEK_SET) != 0) {
        std_fclose(filep);
        return FileIoError();
    }
    size_t nread = std_fread(result.data(), 1, result.size(), filep);
    // Note: cast to unsigned safe because we excluded negative case above
    if ((unsigned long)pos != nread) {
        (void)std_fclose(filep);
        return FileIoError();
    }
    // Note: afaik fclose() should not fail when we're just reading
    if (std_fclose(filep) != 0) {
        return FileIoError();
    }
    return result;
}

template <MK_MOCK_AS(std::fopen, std_fopen),
          MK_MOCK_AS(std::fwrite, std_fwrite),
          MK_MOCK_AS(std::fclose, std_fclose)>
Error overwrite_file_impl(std::string path, std::string content) {
    FILE *filep = std_fopen(path.c_str(), "wb");
    if (filep == nullptr) {
        return FileIoError();
    }
    size_t count = std_fwrite(content.data(), 1, content.size(), filep);
    if (count != content.size()) {
        (void)std_fclose(filep);
        return FileIoError();
    }
    if (std_fclose(filep) != 0) {
        return FileIoError();
    }
    return NoError();
}

inline void timeval_now(timeval *tv) {
    *tv = {};
    if (gettimeofday(tv, nullptr) != 0) {
        throw std::runtime_error("gettimeofday()");
    }
}

inline double time_now() {
    timeval tv;
    timeval_now(&tv);
    double result = tv.tv_sec + tv.tv_usec / (double)1000000.0;
    return result;
}

inline void utc_time_now(struct tm *utc) {
    time_t tv = {};
    tv = time(nullptr);
    gmtime_r(&tv, utc);
}

inline Error parse_iso8601_utc(std::string ts, std::tm *tmb) {
    *tmb = {}; // "portable programs should initialize the structure"
    std::istringstream ss(ts);
    ss >> std::get_time(tmb, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
        return ValueError();
    }
    return NoError();
}

inline ErrorOr<std::string> timestamp(const struct tm *t) {
    char result[30];
    if (strftime(result, sizeof(result), "%Y-%m-%d %H:%M:%S", t) == 0) {
        return ValueError();
    }
    return std::string(result);
}

inline timeval *timeval_init(timeval *tv, double delta) {
    if (delta < 0) {
        return nullptr;
    }
    tv->tv_sec = (time_t)floor(delta);
    tv->tv_usec = (suseconds_t)((delta - floor(delta)) * 1000000);
    return tv;
}

template <typename T=std::list<std::string>>
T split(std::string s, std::string pattern = "\\s+") {
    // See <http://stackoverflow.com/questions/9435385/>
    // passing -1 as the submatch index parameter performs splitting
    std::regex re{pattern};
    std::sregex_token_iterator
        first{s.begin(), s.end(), re, -1},
        last;
    return {first, last};
}

// See <http://stackoverflow.com/questions/440133/>
inline std::string random_within_charset(const std::string charset, size_t length) {
    if (charset.size() < 1) {
        throw ValueError();
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

inline std::string random_printable(size_t length) {
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

inline std::string random_str(size_t length) {
    static const std::string alnum =
            "0123456789"                 // numbers
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ" // uppercase
            "abcdefghijklmnopqrstuvwxyz" // lowercase
        ;
    return random_within_charset(alnum, length);
}

inline std::string random_str_uppercase(size_t length) {
    static const std::string num_upper =
            "0123456789"                  // numbers
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"  // uppercase
        ;
    return random_within_charset(num_upper, length);
}

inline void dump_settings(Settings &s, std::string prefix, Var<Logger> logger) {
    logger->log(MK_LOG_DEBUG2, "%s: {", prefix.c_str());
    for (auto pair : s) {
        logger->log(MK_LOG_DEBUG2, "%s:   \"%s\": \"%s\",", prefix.c_str(),
                    pair.first.c_str(), pair.second.c_str());
    }
    logger->log(MK_LOG_DEBUG2, "%s: }", prefix.c_str());
}

// See: <http://stackoverflow.com/questions/2262386/>
inline std::string sha256_of(std::string input) {
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

inline ErrorOr<std::vector<char>> slurpv(std::string path) {
    return slurpv_impl<char>(path);
}

inline ErrorOr<std::string> slurp(std::string path) {
    ErrorOr<std::vector<char>> v = slurpv_impl<char>(path);
    if (!v) {
        return v.as_error();
    }
    std::string s{v->begin(), v->end()};  /* Note that here we make a copy */
    return s;
}

inline Error overwrite_file(std::string path, std::string content) {
    return overwrite_file_impl(path, content);
}

inline bool startswith(std::string s, std::string p) {
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
inline bool endswith(std::string s, std::string p) {
    return s.size() >= p.size() ? s.rfind(p) == (s.size() - p.size()) : false;
}

inline std::string random_choice(std::vector<std::string> inputs) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(inputs.begin(), inputs.end(), g);
    return inputs[0];
}

inline std::string randomly_capitalize(std::string input) {
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

// Adapted from <http://code.activestate.com/recipes/511478/>
inline double percentile(std::vector<double> v, double percent) {
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

inline double median(std::vector<double> v) {
    return percentile(v, 0.5);
}

} // namespace mk
#endif
