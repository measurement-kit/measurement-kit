// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_QCTHT__HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_QCTHT__HPP

// QCTHT = query class and type helper template

#include <string>
#include <tuple>

namespace mk {
namespace dns {

template <typename Type, Type (*Mapping)(std::string)> class qctht_ {
  public:
    qctht_() noexcept {}

    qctht_(Type v) noexcept : id_{v} {}

    template <typename Stringish> qctht_(Stringish v) { *this = v; }

    qctht_ &operator=(Type v) noexcept {
        id_ = v;
        return *this;
    }

    template <typename Stringish> qctht_ &operator=(Stringish v) {
        id_ = (*Mapping)(v);
        return *this;
    }

    bool operator==(Type id) const noexcept { return id_ == id; }

    bool operator!=(Type id) const noexcept { return id_ != id; }

    operator Type() const noexcept { return id_; }

  private:
    Type id_ = (*Mapping)("");
};

} // namespace dns
} // namespace mk
#endif
