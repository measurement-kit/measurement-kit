/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_COMMON_ERROR_H
# define LIBIGHT_COMMON_ERROR_H
# ifdef __cplusplus

struct IghtError {
	int error = 0;

	IghtError(int e) {
		this->error = e;
	};
};

# endif  /* __cplusplus */
#endif  /* LIBIGHT_COMMON_ERROR_H */
