/*
 * Public domain.
 */
#ifndef MEASUREMENT_KIT_PORTABLE_ERR_H
#define MEASUREMENT_KIT_PORTABLE_ERR_H

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

#endif
