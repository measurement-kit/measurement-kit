/*
 * Public domain.
 */
#ifndef SRC_LIBMEASUREMENT_KIT_PORTABLE_INTERNAL_H
#define SRC_LIBMEASUREMENT_KIT_PORTABLE_INTERNAL_H

#include "../portable/getopt.h"

#include <measurement_kit/portable/time.h>
#include <measurement_kit/portable/err.h>
#include <measurement_kit/portable/stdlib.h>

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void mkp_err(int eval, const char *fmt, ...);
void mkp_errx(int eval, const char *fmt, ...);
void mkp_warn(const char *fmt, ...);
void mkp_warnx(const char *fmt, ...);

extern int mkp_opterr;
extern int mkp_optind;
extern int mkp_optopt;
extern int mkp_optreset;
extern char *mkp_optarg;

int mkp_getopt(int nargc, char * const *nargv, const char *options);

int mkp_getopt_long(int nargc, char * const *nargv, const char *options,
                    const struct option *long_options, int *idx);

int mkp_getopt_long_only(int nargc, char * const *nargv, const char *options,
                         const struct option *long_options, int *idx);

struct tm *mkp_gmtime_r(const time_t *clock, struct tm *result);

int mkp_strcasecmp(const char *s1, const char *s2);

int mkp_strncasecmp(const char *s1, const char *s2, size_t n)t;

long long mkp_strtonum(const char *numstr, long long minval, long long maxval,
                       const char **errstrp);

#endif
