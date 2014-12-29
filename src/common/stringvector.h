/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_STRINGVECTOR_H
# define LIBIGHT_STRINGVECTOR_H
# ifdef __cplusplus

#include "common/constraints.hpp"

/*-
 * StringVector
 *   A vector of strings that is used to implement the resolver.
 */

class IghtPoller;

struct IghtStringVector : public ight::common::constraints::NonCopyable,
		public ight::common::constraints::NonMovable {
    private: 
	char **base;
	size_t count;
	size_t iter;
	size_t pos;
	IghtPoller *poller;
    public:
	IghtStringVector(IghtPoller *, size_t);
	int append(const char *);
	IghtPoller *get_poller(void);
	const char *get_next(void);
	~IghtStringVector(void);
};

# endif  /* __cplusplus */
#endif  /* LIBIGHT_STRINGVECTOR_H */
