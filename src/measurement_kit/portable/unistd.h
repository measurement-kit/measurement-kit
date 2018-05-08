/*
 * Public domain.
 */
#ifndef SRC_MEASUREMENT_KIT_PORTABLE_UNISTD_H
#define SRC_MEASUREMENT_KIT_PORTABLE_UNISTD_H

#if defined _MSC_VER
#include "src/measurement_kit/portable/_getopt.h"
#else
#include <unistd.h>
#endif

#endif
