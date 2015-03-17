/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_COMMON_STRING_VECTOR_HPP
#define IGHT_COMMON_STRING_VECTOR_HPP

#include <ight/common/constraints.hpp>
#include <ight/common/poller.hpp>

/*-
 * StringVector
 *   A vector of strings that is used to implement the resolver.
 */

namespace ight {
namespace common {
namespace string_vector {

using namespace ight::common::constraints;
using namespace ight::common::poller;

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

}}}
#endif
