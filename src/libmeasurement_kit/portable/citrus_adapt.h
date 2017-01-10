/*
 * Public domain.
 */
#ifndef SRC_LIBMEASUREMENT_KIT_PORTABLE_CITRUS_ADAPT_H
#define SRC_LIBMEASUREMENT_KIT_PORTABLE_CITRUS_ADAPT_H

#include <wchar.h>

#ifndef __restrict
# define __restrict /* nothing */
#endif

/*
 * Note: this is to edit the names of the symbols such that the compiled
 * library only contains symbol names starting with `mk_`.
 */
#define _citrus_utf8_ctype_mbrtowc mk_utf8_mbrtowc
#define _citrus_utf8_ctype_mbsinit mk_utf8_mbsinit
#define _citrus_utf8_ctype_mbsnrtowcs mk_utf8_mbsnrtowcs
#define _citrus_utf8_ctype_wcrtomb mk_utf8_wcrtomb
#define _citrus_utf8_ctype_wcsnrtombs mk_utf8_wcsnrtombs

#include "../portable/citrus_ctype.h"

struct _utf8_state {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
};

#endif
