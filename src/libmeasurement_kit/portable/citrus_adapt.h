/*
 * Public domain.
 */
#ifndef SRC_LIBMEASUREMENT_KIT_PORTABLE_CITRUS_ADAPT_H
#define SRC_LIBMEASUREMENT_KIT_PORTABLE_CITRUS_ADAPT_H

#include <wchar.h>

#ifndef __restrict
# define __restrict /* nothing */
#endif

#define mk_utf8_mbrtowc _citrus_utf8_ctype_mbrtowc

#include "../portable/citrus_ctype.h"

struct _utf8_state {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
};

#endif
