// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_REPORT_ENTRY_HPP
#define MEASUREMENT_KIT_REPORT_ENTRY_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/ext.hpp>

namespace mk {
namespace report {

// A report entry.
class Entry : public nlohmann::json {
  public:
    using nlohmann::json::json;

    static Entry array();
    static Entry object();

    template <typename T> operator ErrorOr<T>() {
        try {
            return nlohmann::json::operator T();
        } catch (std::domain_error &) {
            return JsonDomainError();
        }
    }

    // Implementation of dict
    template <typename K> Entry &operator[](const K &key) {
        try {
            return static_cast<Entry &>(nlohmann::json::operator[](key));
        } catch (std::domain_error &) {
            throw JsonDomainError();
        }
    }

    static Entry parse(const std::string &s);

    // Implementation of list
    void push_back(Entry);

    std::string dump();

    bool operator==(std::nullptr_t right);
    bool operator!=(std::nullptr_t right);

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace report
} // namespace mk
#endif
