/*
 * Public domain.
 */
#ifndef MEASUREMENT_KIT_PORTABLE_ERR_H
#define MEASUREMENT_KIT_PORTABLE_ERR_H

#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif

void mkp_err(int eval, const char *fmt, ...);
void mkp_errx(int eval, const char *fmt, ...);
void mkp_warn(const char *fmt, ...);
void mkp_warnx(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#else
#include <err.h>

/* Use translation macros to map to the correct libc functions */
#define mkp_err err
#define mkp_errx errx
#define mkp_warn warn
#define mkp_warnx warnx

#endif // _WIN32/!_WIN32
#endif
