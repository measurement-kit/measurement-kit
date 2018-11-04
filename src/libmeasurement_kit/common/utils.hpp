// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_UTILS_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_UTILS_HPP

#include "src/libmeasurement_kit/common/mock.hpp"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <event2/util.h>
#include <iomanip>
#include <list>
#include <measurement_kit/common.hpp>
#include <openssl/sha.h>
#include <random>

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
ErrorOr<std::vector<T>> slurpv(std::string p) {
    FILE *filep = std_fopen(p.c_str(), "rb");
    if (filep == nullptr) {
        return {FileIoError(), {}};
    }
    if (std_fseek_1(filep, 0, SEEK_END) != 0) {
        std_fclose(filep);
        return {FileIoError(), {}};
    }
    // Note: ftello() might be better for reading very large files but
    // honestly I do think we should use some kind of mmap for them.
    long pos = std_ftell(filep);
    if (pos < 0) {
        std_fclose(filep);
        return {FileIoError(), {}};
    }
    std::vector<T> result;
    // Note: cast to unsigned safe because we excluded negative case above
    result.resize((unsigned long)pos, 0);
    if (std_fseek_2(filep, 0, SEEK_SET) != 0) {
        std_fclose(filep);
        return {FileIoError(), {}};
    }
    size_t nread = std_fread(result.data(), 1, result.size(), filep);
    // Note: cast to unsigned safe because we excluded negative case above
    if ((unsigned long)pos != nread) {
        (void)std_fclose(filep);
        return {FileIoError(), {}};
    }
    // Note: afaik fclose() should not fail when we're just reading
    if (std_fclose(filep) != 0) {
        return {FileIoError(), {}};
    }
    return {NoError(), std::move(result)};
}

template <MK_MOCK_AS(std::fopen, std_fopen),
          MK_MOCK_AS(std::fwrite, std_fwrite),
          MK_MOCK_AS(std::fclose, std_fclose)>
Error overwrite_file(std::string path, std::string content) {
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

#if defined _WIN32
#if defined __MINGW32__
template <MK_MOCK_AS(mingw_gettimeofday, gettimeofday)>
#else
template <MK_MOCK_AS(evutil_gettimeofday, gettimeofday)>
#endif
#else
template <MK_MOCK(gettimeofday)>
#endif
void timeval_now(timeval *tv) {
    *tv = {};
    if (gettimeofday(tv, nullptr) != 0) {
        throw std::runtime_error("gettimeofday()");
    }
}

double time_now();

// TODO(bassosimone): find a better solution, which most likely is
// using the C++11 library to provide this functionality.
#ifdef _WIN32
static inline void utc_time_now(struct tm *utc) {
    time_t tv = {};
    tv = time(nullptr);
    // Note: on Windows gmtime() uses thread local storage. Disable
    // the related warning since we don't store the return value.
    //
    // See <https://stackoverflow.com/a/12060751>.
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif
    auto rv = gmtime(&tv);
#ifdef _MSC_VER
#pragma warning(default:4996)
#endif
    assert(rv != nullptr);
    *utc = *rv;
}
#else
template <MK_MOCK(time), MK_MOCK(gmtime_r)>
void utc_time_now(struct tm *utc) {
    time_t tv = {};
    tv = time(nullptr);
    gmtime_r(&tv, utc);
}
#endif

Error parse_iso8601_utc(std::string ts, std::tm *tmb);

template <MK_MOCK(strftime)>
ErrorOr<std::string> timestamp(const struct tm *t) {
    char result[30];
    // Under Windows systems the "invalid parameter handler" is invoked if
    // `struct tm` has out-of-range parameters. If you ask me, this whole
    // invalid-parameter-handle idea is very suprising since I was expecting
    // C functions to return an error value on failure, not to abort. We
    // should really run away from Microsoft own implementation of libc as
    // it's very weird, and we should instead use as much as possible C++11,
    // which _at least_ is specified and predictable. Until we reach that
    // point, it seems that `tm_mday` is the only parameter for which zero
    // is not a valid value. So use that as an indicator that the struct is
    // not initialized and return an error. ("What a pain" - cit.)
    if (t->tm_mday == 0 || strftime( //
          result, sizeof(result), "%Y-%m-%d %H:%M:%S", t) == 0) {
        return {ValueError(), std::string{}};
    }
    return {NoError(), std::string(result)};
}

timeval *timeval_init(timeval *tv, double delta);

template <typename T=std::list<std::string>>
T split(std::string s, std::string pattern = " ") {
    T res{};
    if (pattern.size() <= 0) {
        res.push_back(std::move(s));
        return res;
    }
    while (!s.empty()) {
        size_t idx = s.find(pattern);
        if (idx == std::string::npos) {
            res.push_back(std::move(s));
            break;
        }
        std::string r = s.substr(0, idx);
        if (!r.empty()) {
            res.push_back(std::move(r));
        }
        s = s.substr(idx + pattern.size());
    }
    return res;
}

std::string random_within_charset(const std::string charset, size_t length);

std::string random_printable(size_t length);

std::string random_str(size_t length);

std::string random_str_uppercase(size_t length);

std::string random_str_lower_alpha(size_t length);

std::string random_tld();

void dump_settings(Settings &s, std::string prefix, SharedPtr<Logger> logger);

std::string sha256_of(std::string input);

ErrorOr<std::string> slurp(std::string path);

bool startswith(std::string s, std::string p);

bool endswith(std::string s, std::string p);

std::string random_choice(std::vector<std::string> inputs);

std::string randomly_capitalize(std::string input);

double percentile(std::vector<double> v, double percent);

double median(std::vector<double> v);

} // namespace mk
#endif
