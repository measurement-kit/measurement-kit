/*
 * Public domain.
 */

#include <measurement_kit/portable/time.h>
#include <stddef.h>

struct tm *gmtime_r(const time_t *clock, struct tm *result) {
    /*
     *     "The MSVC implementation of gmtime() is already thread safe, the
     *      returned struct tm* is allocated in thread-local storage."
     *
     * - http://stackoverflow.com/a/12060751
     */
    if (result == NULL) {
        return NULL;
    }
    struct tm *rval = gmtime(clock);
    if (rval == NULL) {
        return NULL;
    }
    *result = *rval;
    return result;
}
