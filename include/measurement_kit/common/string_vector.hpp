// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_STRING_VECTOR_HPP
#define MEASUREMENT_KIT_COMMON_STRING_VECTOR_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/poller.hpp>

/*-
 * StringVector
 *   A vector of strings that is used to implement the resolver.
 */

namespace measurement_kit {
namespace common {

struct StringVector : public NonCopyable, public NonMovable {
  private:
    char **base;
    size_t count;
    size_t iter;
    size_t pos;
    Poller *poller;

  public:
    StringVector(Poller *, size_t);
    int append(const char *);
    Poller *get_poller(void);
    const char *get_next(void);
    ~StringVector(void);
};

}}
#endif
