// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_UTILS_HPP
#define PRIVATE_COMMON_UTILS_HPP

#include "private/common/mock.hpp"
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

template <MK_MOCK(gettimeofday)> void timeval_now(timeval *tv) {
    *tv = {};
    if (gettimeofday(tv, nullptr) != 0) {
        throw std::runtime_error("gettimeofday()");
    }
}

double time_now();

template <MK_MOCK(time), MK_MOCK(gmtime_r)>
void utc_time_now(struct tm *utc) {
    time_t tv = {};
    tv = time(nullptr);
    gmtime_r(&tv, utc);
}

Error parse_iso8601_utc(std::string ts, std::tm *tmb);

template <MK_MOCK(strftime)>
ErrorOr<std::string> timestamp(const struct tm *t) {
    char result[30];
    if (strftime(result, sizeof(result), "%Y-%m-%d %H:%M:%S", t) == 0) {
        return ValueError();
    }
    return std::string(result);
}

timeval *timeval_init(timeval *tv, double delta);

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
