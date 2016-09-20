/*
 * Public domain.
 */
#ifndef SRC_LIBMEASUREMENT_KIT_PORTABLE_API_H
#define SRC_LIBMEASUREMENT_KIT_PORTABLE_API_H

/*
 * The general idea is that throughout the code we use functions prefixed
 * by "mkp" (for MK portable). Then we include this file. This file checks
 * what is actually implemented by the platform and gives preference to
 * the platform. For now, we compile our mkp functions in any case to make
 * sure that they compile cleanly on all platform interesting to us.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../portable/internal.h"

#ifdef HAVE_ERR
#define mkp_err err
#endif

#ifdef HAVE_ERRX
#define mkp_errx errx
#endif

#ifdef HAVE_WARN
#define mkp_warn warn
#endif

#ifdef HAVE_WARNX
#define mkp_warnx warnx
#endif

#ifdef HAVE_GETOPT
#define mkp_opterr opterr
#define mkp_optind optind
#define mkp_optopt optopt
#define mkp_optreset optreset
#define mkp_optarg optarg
#define mkp_getopt getopt
#endif

#ifdef HAVE_GETOPT_LONG
#define mkp_getopt_long getopt_long
#endif

#ifdef HAVE_GETOPT_LONG_ONLY
#define mkp_getopt_long_only getopt_long_only
#endif

#ifdef HAVE_GMTIME_R
#define mkp_gmtime_r gmtime_r
#endif

#ifdef HAVE_STRCASECMP
#define mkp_strcasecmp strcasecmp
#endif

#ifdef HAVE_STRNCASECMP
#define mkp_strncasecmp strncasecmp
#endif

#ifdef HAVE_STRTONUM
#define mkp_strtonum strtonum
#endif

#endif
