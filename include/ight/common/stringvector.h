/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_STRINGVECTOR_H
# define LIBIGHT_STRINGVECTOR_H
# ifdef __cplusplus

#include <ight/common/constraints.hpp>
#include <ight/common/poller.h>

/*-
 * StringVector
 *   A vector of strings that is used to implement the resolver.
 */

struct IghtStringVector : public ight::common::constraints::NonCopyable,
		public ight::common::constraints::NonMovable {
    private: 
	char **base;
	size_t count;
	size_t iter;
	size_t pos;
	ight::common::poller::Poller *poller;
    public:
	IghtStringVector(ight::common::poller::Poller *, size_t);
	int append(const char *);
	ight::common::poller::Poller *get_poller(void);
	const char *get_next(void);
	~IghtStringVector(void);
};

# endif  /* __cplusplus */
#endif  /* LIBIGHT_STRINGVECTOR_H */
