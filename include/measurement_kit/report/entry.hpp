// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_REPORT_ENTRY_HPP
#define MEASUREMENT_KIT_REPORT_ENTRY_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/ext/json.hpp> // vendored nlohmann::json

namespace mk {
namespace report {

// A report entry.
class Entry : public nlohmann::json {
  public:
    Entry() : nlohmann::json() {}
    template <typename T> Entry(T t) : nlohmann::json(t) {}

    /*
     * I'd like to also declare these two constructors but apparently
     * there are cases where having all constructors defined create
     * some ambiguity and I don't know how to avoid that.
     *
     * Leaving this comment here so maybe someone can teach me.
     */
    //template <typename T> Entry(T &t) : nlohmann::json(t) {}
    //template <typename T> Entry(T &&t) : nlohmann::json(t) {}

    Entry(std::initializer_list<nlohmann::json> nl) : nlohmann::json(nl) {}

    Entry(Entry &) = default;
    Entry(const Entry &) = default;
    Entry(Entry &&) = default;

    static Entry array();
    static Entry object();

    template <typename T> Entry &operator=(T t) {
        nlohmann::json::operator=(t);
        return *this;
    }

    Entry &operator=(std::initializer_list<nlohmann::json> t);
    Entry &operator=(Entry &) = default;
    Entry &operator=(Entry &&) = default;

    template <typename T> operator ErrorOr<T>() {
        try {
            return nlohmann::json::operator T();
        } catch (const std::domain_error &) {
            return JsonDomainError();
        }
    }

    // Implementation of dict
    template <typename K> Entry &operator[](const K &key) {
        try {
            return static_cast<Entry &>(nlohmann::json::operator[](key));
        } catch (const std::domain_error &) {
            throw JsonDomainError();
        }
    }

    static Entry parse(const std::string &s);

    // Implementation of list

    void push_back(Entry);

    template <typename T> void push_back(T t) {
        try {
            nlohmann::json::push_back(t);
        } catch (const std::domain_error &) {
            throw JsonDomainError();
        }
    }

    void push_back(std::initializer_list<nlohmann::json> j);

    std::string dump(const int indent = -1) const;

    bool operator==(std::nullptr_t right) const noexcept;
    bool operator!=(std::nullptr_t right) const noexcept;

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace report
} // namespace mk
#endif
