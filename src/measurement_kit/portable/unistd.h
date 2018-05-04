/*
 * Public domain.
 */
#ifndef SRC_MEASUREMENT_KIT_PORTABLE_UNISTD_H
#define SRC_MEASUREMENT_KIT_PORTABLE_UNISTD_H

#ifndef _WIN32
#include <unistd.h>
#else
#include "src/measurement_kit/portable/_getopt.h"
#endif

#endif
