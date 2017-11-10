/*-
 * Part of measurement-kit <https://measurement-kit.github.io/>.
 * Measurement-kit is free software under the BSD license. See AUTHORS
 * and LICENSE for more information on the copying conditions.
 * =============================================================
 * Based on $OpenBSD: citrus_ctype.h,v 1.5 2016/09/05 09:47:02 schwarze Exp $
 *
 * Portions Copyright (c)2002 Citrus Project,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef PRIVATE_COMMON_CITRUS_CTYPE_H
#define PRIVATE_COMMON_CITRUS_CTYPE_H

#include <wchar.h>

#ifndef __restrict
# define __restrict /* nothing */
#endif

#define _CITRUS_UTF8_MB_CUR_MAX 4

struct _utf8_state {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
};

#ifdef __cplusplus
extern "C" {
#endif

size_t	mk_utf8_mbrtowc(wchar_t * __restrict,
		const char * __restrict, size_t, mbstate_t * __restrict);
int     mk_utf8_mbsinit(const mbstate_t * __restrict);
size_t  mk_utf8_mbsnrtowcs(wchar_t * __restrict,
		const char ** __restrict, size_t, size_t,
		mbstate_t * __restrict);
size_t  mk_utf8_wcrtomb(char * __restrict, wchar_t,
		mbstate_t * __restrict);
size_t  mk_utf8_wcsnrtombs(char * __restrict,
		const wchar_t ** __restrict, size_t, size_t,
		mbstate_t * __restrict);

#ifdef __cplusplus
}
#endif
#endif
