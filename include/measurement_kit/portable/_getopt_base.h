/*
 * Public domain
 */
#ifndef MEASUREMENT_KIT_PORTABLE_GETOPT_BASE_H
#define MEASUREMENT_KIT_PORTABLE_GETOPT_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

int getopt(int argc, char *const argv[], const char *optstring);
#endif

/*
 * This header should be included either by unistd.h or by getopt.h.
 */
#if (defined MKP_UNISTD_VISIBLE || defined MKP_GETOPT_VISIBLE)

extern int mkp_opterr;
extern int mkp_optind;
extern int mkp_optopt;
extern int mkp_optreset;
extern char *mkp_optarg;

int mkp_getopt(int nargc, char * const *nargv, const char *options);

#endif
#ifdef __cplusplus
}
#endif
#endif
