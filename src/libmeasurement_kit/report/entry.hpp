// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_REPORT_ENTRY_HPP
#define SRC_LIBMEASUREMENT_KIT_REPORT_ENTRY_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace report {

// A report entry.
class Entry : public Json {
  public:
    Entry() : Json() {}
    template <typename T> Entry(T t) : Json(t) {}

    /*
     * I'd like to also declare these two constructors but apparently
     * there are cases where having all constructors defined create
     * some ambiguity and I don't know how to avoid that.
     *
     * Leaving this comment here so maybe someone can teach me.
     */
    //template <typename T> Entry(T &t) : Json(t) {}
    //template <typename T> Entry(T &&t) : Json(t) {}

    Entry(std::initializer_list<Json> nl) : Json(nl) {}

    Entry(Entry &) = default;
    Entry(const Entry &) = default;
    Entry(Entry &&) = default;

    static Entry array();
    static Entry object();

    template <typename T> Entry &operator=(T t) {
        Json::operator=(t);
        return *this;
    }

    Entry &operator=(std::initializer_list<Json> t);
    Entry &operator=(Entry &) = default;
    Entry &operator=(Entry &&) = default;

    template <typename T> operator ErrorOr<T>() {
        try {
            return {NoError(), Json::operator T()};
        } catch (const std::domain_error &) {
            return {JsonDomainError(), {}};
        }
    }

    // Implementation of dict
    template <typename K> Entry &operator[](const K &key) {
        try {
            return static_cast<Entry &>(Json::operator[](key));
        } catch (const std::domain_error &) {
            throw JsonDomainError();
        }
    }

    static Entry parse(const std::string &s);

    // Implementation of list

    void push_back(Entry);

    template <typename T> void push_back(T t) {
        try {
            Json::push_back(t);
        } catch (const std::domain_error &) {
            throw JsonDomainError();
        }
    }

    void push_back(std::initializer_list<Json> j);

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
