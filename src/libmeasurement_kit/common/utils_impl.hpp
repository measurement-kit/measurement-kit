// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_UTILS_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_UTILS_IMPL_HPP

#include "../common/utils.hpp"
#include <cstdio>
#include <measurement_kit/common.hpp>

namespace mk {

/*
 * There are many ways in C++ to slurp the whole content of a file into a
 * C++ string. The following is my favourite. Compared to other methods,
 * not only this seems fastest, but also it provides greater control over
 * errors that can occur. Thus mocking is simpler.
 *
 * See <http://stackoverflow.com/questions/116038/what-is-the-best-way-to-read-an-entire-file-into-a-stdstring-in-c>.
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

} // namespace mk
#endif
