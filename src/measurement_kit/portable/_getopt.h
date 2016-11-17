/*
 * Public domain.
 */
#ifndef SRC_MEASUREMENT_KIT_PORTABLE__GETOPT_H
#define SRC_MEASUREMENT_KIT_PORTABLE__GETOPT_H

#if (!defined SRC_MEASUREMENT_KIT_PORTABLE_UNISTD_H && \
     !defined SRC_MEASUREMENT_KIT_PORTABLE_GETOPT_H)
# error "measurement_kit/portable/_getopt.h included from unknown header"
#endif

#ifndef _WIN32

/* If we're not under Windows, we are in a system where getopt() is
   defined and hence just convert our portability names to the names
   that are actually used under such system */
#define mkp_optarg optarg
#define mkp_optind optind
#define mkp_opterr opterr
#define mkp_optreset optreset
#define mkp_getopt getopt

#endif // !_WIN32

#endif
