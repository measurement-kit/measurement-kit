/*
 * Public domain.
 */
#ifndef SRC_LIBMEASUREMENT_KIT_PORTABLE_CITRUS_ADAPT_H
#define SRC_LIBMEASUREMENT_KIT_PORTABLE_CITRUS_ADAPT_H

#include <wchar.h>

#ifndef __restrict
# define __restrict /* nothing */
#endif

#define _citrus_utf8_ctype_mbrtowc mk_citrus_utf8_ctype_mbrtowc
#define _citrus_utf8_ctype_mbsinit mk_citrus_utf8_ctype_mbsinit
#define _citrus_utf8_ctype_mbsnrtowcs mk_citrus_utf8_ctype_mbsnrtowcs
#define _citrus_utf8_ctype_wcrtomb mk_citrus_utf8_ctype_wcrtomb
#define _citrus_utf8_ctype_wcsnrtombs mk_citrus_utf8_ctype_wcsnrtombs

#include "../portable/citrus_ctype.h"

struct _utf8_state {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
};

#endif
